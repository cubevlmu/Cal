#pragma once

#include "base/types/Span.hpp"
#include <globals.hpp>

namespace cal::platform {

    typedef enum : u32 {
        Ok = 0, Warn = 1, Error = 2
    } MsgBox_;
    typedef u32 MsgBoxOption;

    void init();
    void abort();
    void logInfo();

    void* memReserve(size_t size);
    void memCommit(void* ptr, size_t size);
    void memRelease(void* ptr, size_t size); // size must be full size used in reserve
    u32 getMemPageSize();
    u32 getMemPageAlignment();
    u64 getProcessMemory();


    void messageBox(const char* text, MsgBoxOption opt = 0);
    void setCommandLine(int, char**);
    bool getCommandLine(Span<char> output);
    [[nodiscard]] bool getOpenFilename(Span<char> out, const char* filter, const char* starting_file);
    [[nodiscard]] bool getSaveFilename(Span<char> out, const char* filter, const char* default_extension);
    [[nodiscard]] bool getOpenDirectory(Span<char> out, const char* starting_dir);


    void* loadLibrary(const char* path);
    void unloadLibrary(void* handle);
    void* getLibrarySymbol(void* handle, const char* name);
    float getTimeSinceProcessStart();
}