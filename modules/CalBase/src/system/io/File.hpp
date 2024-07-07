#pragma once

#include "globals.hpp"
#include "base/types/Span.hpp"

namespace cal {

    template <typename T> struct Action;
    template <typename T> struct UniquePtr;

    namespace platform {
        struct FileIterator;
        struct IFile;
        struct OFile;
    }


    struct CAL_API File {
        using ByteArray = Span<const u8>;
        using ContentCallback = Action<void(ByteArray, bool)>;

        struct CAL_API AsyncFileHandler
        {
            static AsyncFileHandler invalid()
            {
                return AsyncFileHandler(0xffFFffFF);
            }
            explicit AsyncFileHandler(u32 v) : value(v) {}
            u32 value;
            bool isValid() const { return value != 0xffFFffFF; }
        };

        static UniquePtr<File> create(const char* base_path, struct IAllocator& a);
        static UniquePtr<File> createPacked(const char* pak_path, struct IAllocator& a);

        virtual ~File() = default;

        virtual u64 getLastModified(struct StringView path) = 0;
        virtual bool copyFile(StringView from, StringView to) = 0;
        virtual bool moveFile(StringView from, StringView to) = 0;
        virtual bool deleteFile(StringView path) = 0;
        virtual bool fileExists(StringView path) = 0;
        virtual platform::FileIterator* createFileIterator(StringView dir) = 0;
        virtual bool open(StringView path, platform::IFile& file) = 0;
        virtual bool open(StringView path, platform::OFile& file) = 0;

        virtual void setBasePath(const char* path) = 0;
        virtual const char* getBasePath() const = 0;
        virtual void processCallbacks() = 0;
        virtual bool hasWork() = 0;

        [[nodiscard]] virtual bool saveContentSync(
            const struct Path& file, Span<const u8> content
        ) = 0;
        [[nodiscard]] virtual bool getContentSync(
            const struct Path& file, struct MemoryOStream& content
        ) = 0;
        virtual AsyncFileHandler getContent(
            const Path& file, const ContentCallback& callback
        ) = 0;
        virtual void cancel(AsyncFileHandler handle) = 0;
    };
}