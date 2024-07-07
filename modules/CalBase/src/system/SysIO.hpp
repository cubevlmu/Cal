#pragma once

#include "globals.hpp"
#include "system/io/Stream.hpp"

namespace cal::platform {

    struct CAL_API IFile final : IIStream {
        IFile();
        ~IFile();

        [[nodiscard]] bool open(const char* path);
        void close();

        using IIStream::read;
        [[nodiscard]] bool read(void* data, u64 size) override;

        u64 size() const override;
        u64 pos();

        [[nodiscard]] bool seek(u64 pos);

    private:
        void* m_handle;
    };


    struct CAL_API OFile final : IOStream {
        OFile();
        ~OFile();

        [[nodiscard]] bool open(const char* path);
        void close();
        void flush();
        bool isError() const { return m_is_error; }

        using IOStream::write;
        [[nodiscard]] bool write(const void* data, u64 size) override;

    private:
        OFile(const OFile&) = delete;
        void* m_handle;
        bool m_is_error;
    };


    struct FileInfo {
        bool is_directory;
        char filename[MAX_PATH];
    };

    struct FileIterator;



    CAL_API FileIterator* createFileIterator(StringView path, IAllocator& allocator); //DONE
    CAL_API void destroyFileIterator(FileIterator* iterator); //DONE
    CAL_API bool getNextFile(FileIterator* iterator, FileInfo* info); //DONE

    CAL_API void setCurrentDirectory(StringView path); //DONE
    CAL_API void getCurrentDirectory(Span<char> path); //DONE
    CAL_API void copyToClipboard(const char* text); //DONE

    CAL_API bool deleteFile(StringView path); //DONE
    CAL_API [[nodiscard]] bool moveFile(StringView from, StringView to); //DONE
    CAL_API size_t getFileSize(StringView path); //DONE
    CAL_API bool fileExists(StringView path); //DONE
    CAL_API bool dirExists(StringView path); //DONE
    CAL_API u64 getLastModified(StringView file); //DONE
    CAL_API [[nodiscard]] bool makePath(const char* path); //DONE

    CAL_API [[nodiscard]] bool copyFile(StringView from, StringView to); //DONE
    CAL_API void getExecutablePath(Span<char> path); //DONE
    CAL_API [[nodiscard]] bool getAppDataDir(Span<char> path); //DONE
}