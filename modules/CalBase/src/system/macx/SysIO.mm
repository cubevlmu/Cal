#ifdef __MACH__

#include "base/Logger.hpp"
#include "base/types/String.hpp"
#include "system/SysIO.hpp"
#import <Cocoa/Cocoa.h>

namespace cal::platform {

    void getExecutablePath(Span<char> buffer) {
        @autoreleasepool {
            NSString *executablePath = [[NSProcessInfo processInfo] arguments][0];
           
            string::copyString(buffer, [executablePath fileSystemRepresentation]);
        }
    }

    
    void copyToClipboard(const char* text) {
        @autoreleasepool {
            NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
            NSString *textToCopy = [NSString stringWithUTF8String:(text)];
            
            [pasteboard clearContents];
            [pasteboard setString:textToCopy forType:NSPasteboardTypeString];

            LogDebug("[System] Copied to clipboard ", text);
        }
    }
}

#endif