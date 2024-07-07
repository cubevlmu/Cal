#ifdef __MACH__

#include "system/SysIO.hpp"
#include "base/types/String.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sys/fcntl.h>

namespace cal::platform {

    IFile::IFile() {
        m_handle = nullptr;
        static_assert(sizeof(m_handle) >= sizeof(FILE*), "");
    }


    OFile::OFile() {
        m_is_error = false;
        m_handle = nullptr;
        static_assert(sizeof(m_handle) >= sizeof(FILE*), "");
    }


    IFile::~IFile() {
        ASSERT(!m_handle);
    }


    OFile::~OFile() {
        ASSERT(!m_handle);
    }


    bool OFile::open(const char* path) {
        m_handle = fopen(path, "wb");
        m_is_error = !m_handle;
        return !m_is_error;
    }


    bool IFile::open(const char* path) {
        m_handle = fopen(path, "rb");
        return m_handle;
    }


    void OFile::flush() {
        ASSERT(m_handle);
        fflush((FILE*)m_handle);
    }


    void OFile::close() {
        if (m_handle) {
            fclose((FILE*)m_handle);
            m_handle = nullptr;
        }
    }


    void IFile::close() {
        if (m_handle) {
            fclose((FILE*)m_handle);
            m_handle = nullptr;
        }
    }


    bool OFile::write(const void* data, u64 size) {
        ASSERT(m_handle);
        const size_t written = fwrite(data, size, 1, (FILE*)m_handle);
        return written == 1;
    }

    bool IFile::read(void* data, u64 size) {
        ASSERT(nullptr != m_handle);
        size_t read = fread(data, size, 1, (FILE*)m_handle);
        return read == 1;
    }

    u64 IFile::size() const {
        ASSERT(nullptr != m_handle);
        long pos = ftell((FILE*)m_handle);
        fseek((FILE*)m_handle, 0, SEEK_END);
        size_t size = (size_t)ftell((FILE*)m_handle);
        fseek((FILE*)m_handle, pos, SEEK_SET);
        return size;
    }


    u64 IFile::pos() {
        ASSERT(nullptr != m_handle);
        long pos = ftell((FILE*)m_handle);
        return (size_t)pos;
    }


    bool IFile::seek(u64 pos) {
        ASSERT(nullptr != m_handle);
        return fseek((FILE*)m_handle, pos, SEEK_SET) == 0;
    }


    struct FileIterator {};

    FileIterator* createFileIterator(StringView _path, IAllocator& allocator) {
        char path[MAX_PATH];
        string::copyString(path, _path);

        return (FileIterator*)opendir(path);
    }


    void destroyFileIterator(FileIterator* iterator) {
        closedir((DIR*)iterator);
    }


    bool getNextFile(FileIterator* iterator, FileInfo* info) {
        if (!iterator) return false;

        auto* dir = (DIR*)iterator;
        auto* dir_ent = readdir(dir);
        if (!dir_ent) return false;

        info->is_directory = dir_ent->d_type == DT_DIR;
        string::copyString(info->filename, dir_ent->d_name);
        return true;
    }


    void setCurrentDirectory(StringView path) {
        std::string cpd(path.begin, path.end);
        auto res = chdir(cpd.c_str());
        (void)res;
    }


    void getCurrentDirectory(Span<char> output) {
        if (!getcwd(output.m_begin, output.length())) {
            output[0] = 0;
        }
    }


    bool deleteFile(StringView path) {
        char tmp[MAX_PATH];
        string::copyString(tmp, path);
        return unlink(tmp) == 0;
    }


    bool moveFile(StringView _from, StringView _to) {
        char from[MAX_PATH];
        string::copyString(from, _from);
        char to[MAX_PATH];
        string::copyString(to, _to);

        return rename(from, to) == 0;
    }


    size_t getFileSize(StringView path) {
        char path_tmp[MAX_PATH];
        string::copyString(path_tmp, path);
        struct stat tmp;
        stat(path_tmp, &tmp);
        return tmp.st_size;
    }


    bool fileExists(StringView _path) {
        char path[MAX_PATH];
        string::copyString(path, _path);
        struct stat tmp;
        return ((stat(path, &tmp) == 0) && (((tmp.st_mode) & S_IFMT) != S_IFDIR));
    }


    bool dirExists(StringView _path) {
        char path[MAX_PATH];
        string::copyString(path, _path);
        struct stat tmp;
        return ((stat(path, &tmp) == 0) && (((tmp.st_mode) & S_IFMT) == S_IFDIR));
    }



    u64 getLastModified(StringView _path) {
        char path[MAX_PATH];
        string::copyString(path, _path);

        struct stat tmp;
        u64 ret = 0;
        if (stat(path, &tmp) != 0) return 0;
        ret = tmp.st_mtimespec.tv_sec * 1000 + u64(tmp.st_mtimespec.tv_nsec / 1000000);
        return ret;
    }


    bool makePath(const char* path) {
        char tmp[MAX_PATH];
        const char* cin = path;
        char* cout = tmp;

        while (*cin) {
            *cout = *cin;
            if (*cout == '\\' || *cout == '/' || *cout == '\0') {
                if (cout != tmp) {
                    *cout = '\0';
                    mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    *cout = *cin;
                }
            }
            ++cout;
            ++cin;
        }
        int res = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        return res == 0 || (res == -1 && errno == EEXIST);
    }


    bool copyFile(StringView from, StringView to) {
        char tmp[MAX_PATH];
        string::copyString(tmp, from);

        const int source = open(tmp, O_RDONLY, 0);
        if (source < 0) return false;

        string::copyString(tmp, to);
        const int dest = open(tmp, O_WRONLY | O_CREAT, 0644);
        if (dest < 1) {
            ::close(source);
            return false;
        }

        char buf[BUFSIZ];
        size_t size;
        while ((size = ::read(source, buf, BUFSIZ)) > 0) {
            const ssize_t res = ::write(dest, buf, size); //-V512
            if (res == -1) {
                ::close(source);
                ::close(dest);
                return false;
            }
        }

        ::close(source);
        ::close(dest);
        return true;
    }


    bool getAppDataDir(Span<char> path) {
        char* home = getenv("HOME");
        if (!home) return false;
        string::copyString(path, home);
        string::catString(path, "/.toy/");
        return true;
    }
}

#endif