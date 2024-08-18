#include "File.hpp"

#include "Stream.hpp"
#include "Path.hpp"
#include "base/Logger.hpp"
#include "base/threading/SyncMutex.hpp"
#include "base/threading/SyncSemaphore.hpp"
#include "base/threading/Thread.hpp"
#include "base/types/container/HashMap.hpp"
#include "system/SysIO.hpp"
#include "system/SysTimer.hpp"
#include "base/threading/Action.hpp"

namespace cal {

    struct AsyncItem {
        enum class Flags : u32 {
            NONE = 0,
            FAILED = 1 << 0,
            CANCELED = 1 << 1,
        };

        AsyncItem(IAllocator& allocator) : data(allocator) {}

        bool isFailed() const { return isFlagSet(flags, Flags::FAILED); }
        bool isCanceled() const { return isFlagSet(flags, Flags::CANCELED); }

        File::ContentCallback callback;
        MemoryOStream data;
        Path path;
        u32 id = 0;
        Flags flags = Flags::NONE;
    };

    struct FileSystemImpl;


    struct FSTask final : Thread {
        FSTask(FileSystemImpl& fs, IAllocator& allocator)
            : Thread(allocator)
            , m_fs(fs)
        {}

        ~FSTask() = default;

        void stop();
        int run() override;

    private:
        FileSystemImpl& m_fs;
        bool m_finish = false;
    };


    struct FileSystemImpl : File {
        explicit FileSystemImpl(const char* base_path, IAllocator& allocator)
            : m_allocator(allocator)
            , m_queue(allocator)
            , m_finished(allocator)
            , m_last_id(0)
            , m_semaphore(0, 0xffFF)
        {
            setBasePath(base_path);
            m_task.create(*this, m_allocator);
            m_task->create("Filesystem", true);
        }

        ~FileSystemImpl() override {
            m_task->stop();
            m_task->destroy();
            m_task.destroy();
        }


        bool hasWork() override
        {
            return m_work_counter != 0;
        }

        const char* getBasePath() const override { return m_base_path; }

        void setBasePath(const char* dir) final
        {
            Path::normalize(dir, m_base_path.data);
            if (!string::endsWith(m_base_path, "/") && !string::endsWith(m_base_path, "\\")) {
                m_base_path.append('/');
            }
        }

        bool saveContentSync(const Path& path, Span<const u8> content) override {
            cal::platform::OFile file;
            const Path full_path(m_base_path, path);
            if (!file.open(full_path.c_str())) return false;

            bool res = file.write(content.begin(), content.length());
            file.close();

            return res;
        }

        bool getContentSync(const Path& path, MemoryOStream& content) override {
            cal::platform::IFile file;
            const Path full_path(m_base_path, path);

            if (!file.open(full_path.c_str())) return false;

            content.resize(file.size());
            if (!file.read(content.getMutableData(), content.size())) {
                LogError("Could not read ", path);
                file.close();
                return false;
            }
            file.close();
            return true;
        }


        AsyncFileHandler getContent(const Path& file, const ContentCallback& callback) override
        {
            if (file.isEmpty()) return AsyncFileHandler::invalid();

            MutexGuard lock(m_mutex);
            ++m_work_counter;
            AsyncItem& item = m_queue.emplace(m_allocator);
            ++m_last_id;
            if (m_last_id == 0) ++m_last_id;
            item.id = m_last_id;
            item.path = file.c_str();
            item.callback = callback;
            m_semaphore.signal();
            return AsyncFileHandler(item.id);
        }


        void cancel(AsyncFileHandler async) override
        {
            MutexGuard lock(m_mutex);
            for (AsyncItem& item : m_queue) {
                if (item.id == async.value) {
                    item.flags |= AsyncItem::Flags::CANCELED;
                    --m_work_counter;
                    return;
                }
            }
            for (AsyncItem& item : m_finished) {
                if (item.id == async.value) {
                    item.flags |= AsyncItem::Flags::CANCELED;
                    return;
                }
            }
            ASSERT(false);
        }


        bool open(StringView path, platform::IFile& file) override
        {
            const Path full_path(m_base_path, path);
            return file.open(full_path.c_str());
        }


        bool open(StringView path, platform::OFile& file) override
        {
            const Path full_path(m_base_path, path);
            return file.open(full_path.c_str());
        }


        bool deleteFile(StringView path) override
        {
            const Path full_path(m_base_path, path);
            return platform::deleteFile(full_path.c_str());
        }


        bool moveFile(StringView from, StringView to) override
        {
            const Path full_path_from(m_base_path, from);
            const Path full_path_to(m_base_path, to);
            return platform::moveFile(full_path_from, full_path_to);
        }


        bool copyFile(StringView from, StringView to) override
        {
            const Path full_path_from(m_base_path, from);
            const Path full_path_to(m_base_path, to);
            return platform::copyFile(full_path_from, full_path_to);
        }


        bool fileExists(StringView path) override
        {
            const Path full_path(m_base_path, path);
            return platform::fileExists(full_path);
        }


        u64 getLastModified(StringView path) override
        {
            const Path full_path(m_base_path, path);
            return platform::getLastModified(full_path);
        }


        platform::FileIterator* createFileIterator(StringView dir) override
        {
            const Path path(m_base_path, dir);
            return platform::createFileIterator(path, m_allocator);
        }

        void processCallbacks() override
        {
            platform::Timer timer;
            for (;;) {
                m_mutex.enter();
                if (m_finished.empty()) {
                    m_mutex.exit();
                    break;
                }

                AsyncItem item = static_cast<AsyncItem&&>(m_finished[0]);
                m_finished.erase(0);
                ASSERT(m_work_counter > 0);
                --m_work_counter;

                m_mutex.exit();

                if (!item.isCanceled()) {
                    item.callback.invoke(Span((const u8*)item.data.data(), (u32)item.data.size()), !item.isFailed());
                }

                if (timer.getTimeSinceStart() > 0.1f) {
                    break;
                }
            }
        }

        IAllocator& m_allocator;
        Local<FSTask> m_task;
        StaticString<MAX_PATH> m_base_path;
        Array<AsyncItem> m_queue;
        u32 m_work_counter = 0;
        Array<AsyncItem> m_finished;
        Mutex m_mutex;
        Semaphore m_semaphore;

        u32 m_last_id;
    };


    int FSTask::run()
    {
        while (!m_finish) {
            m_fs.m_semaphore.wait();
            if (m_finish) break;

            Path path;
            {
                MutexGuard lock(m_fs.m_mutex);
                ASSERT(!m_fs.m_queue.empty());
                path = m_fs.m_queue[0].path;
                if (m_fs.m_queue[0].isCanceled()) {
                    m_fs.m_queue.erase(0);
                    continue;
                }
            }

            MemoryOStream data(m_fs.m_allocator);
            bool success = m_fs.getContentSync(path, data);

            {
                MutexGuard lock(m_fs.m_mutex);
                if (!m_fs.m_queue[0].isCanceled()) {
                    m_fs.m_finished.emplace(static_cast<AsyncItem&&>(m_fs.m_queue[0]));
                    m_fs.m_finished.back().data = static_cast<MemoryOStream&&>(data);
                    if (!success) {
                        m_fs.m_finished.back().flags |= AsyncItem::Flags::FAILED;
                    }
                }
                m_fs.m_queue.erase(0);
            }
        }
        return 0;
    }


    void FSTask::stop()
    {
        m_finish = true;
        m_fs.m_semaphore.signal();
    }

    struct PackFileSystem : FileSystemImpl {
        PackFileSystem(const char* pak_path, IAllocator& allocator)
            : FileSystemImpl("pack://", allocator)
            , m_map(allocator)
        {
            if (!m_file.open(pak_path)) {
                LogError("Failed to open game.pak");
                return;
            }
            const u32 count = m_file.read<u32>();
            for (u32 i = 0; i < count; ++i) {
                const FilePathHash hash = m_file.read<FilePathHash>();
                PackFile& f = m_map.insert(hash);
                f.offset = m_file.read<u64>();
                f.size = m_file.read<u64>();
            }
        }

        ~PackFileSystem() {
            m_file.close();
        }

        bool getContentSync(const Path& path, MemoryOStream& content) override {
            ASSERT(content.size() == 0);
            StringView basename = Path::getBasename(path);
            u64 hashu64;
            string::fromCString(basename, hashu64);
            FilePathHash hash = FilePathHash::fromU64(hashu64);
            if (basename[0] < '0' || basename[0] > '9' || hashu64 == 0) {
                hash = path.getHash();
            }
            auto iter = m_map.find(hash);
            if (!iter.isValid()) {
                iter = m_map.find(path.getHash());
                if (!iter.isValid()) return false;
            }

            content.resize(iter.value().size);
            MutexGuard lock(m_mutex);
            const u32 header_size = sizeof(u32) + m_map.size() * (2 * sizeof(u64) + sizeof(FilePathHash));
            if (!m_file.seek(iter.value().offset + header_size) || !m_file.read(content.getMutableData(), content.size())) {
                LogError("Could not read ", path);
                return false;
            }

            return true;
        }

        struct PackFile {
            u64 offset;
            u64 size;
        };

        HashMap<FilePathHash, PackFile> m_map;
        platform::IFile m_file;
    };


    UniquePtr<File> File::create(const char* base_path, IAllocator& allocator)
    {
        return UniquePtr<FileSystemImpl>::create(allocator, base_path, allocator);
    }


    UniquePtr<File> File::createPacked(const char* pak_path, IAllocator& allocator)
    {
        return UniquePtr<PackFileSystem>::create(allocator, pak_path, allocator);
    }


}