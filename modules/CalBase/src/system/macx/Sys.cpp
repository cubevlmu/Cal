#include "base/types/String.hpp"
#ifdef __MACH__

#include "system/Sys.hpp"

#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>

namespace cal::platform {

    static struct {
        int argc;
        char** argv;
    } G;

    void setCommandLine(int a, char** b) {
        G.argc = a;
        G.argv = b;
    }


    bool getCommandLine(Span<char> output) {
        string::copyString(output, "");
        for (int i = 0; i < G.argc; ++i) {
            string::catString(output, G.argv[i]);
            string::catString(output, " ");
        }
        return true;
    }


    void* memReserve(size_t size) {
        //TODO Test It MAP_ANON   :  MAP_ANONYMOUS |
        void* mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        ASSERT(mem);
        return mem;
    }


    void memCommit(void* ptr, size_t size) {
        // noop on mach
    }


    void memRelease(void* ptr, size_t size) {
        munmap(ptr, size);
    }


    u32 getMemPageSize() {
        const u32 sz = sysconf(_SC_PAGESIZE);
        return sz;
    }

    u32 getMemPageAlignment() {
        return getMemPageSize();
    }

    u64 getProcessMemory()
    {
        return 0;
    }

    void* loadLibrary(const char* path) {
        return dlopen(path, RTLD_LOCAL | RTLD_LAZY);
    }

    void unloadLibrary(void* handle) {
        if (handle != NULL) dlclose(handle);
    }


    void* getLibrarySymbol(void* handle, const char* name) {
        return dlsym(handle, name);
    }


    float getTimeSinceProcessStart() { return -1; }

}

#endif