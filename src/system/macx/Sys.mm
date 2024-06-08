#ifdef __MACH__

#include "base/types/String.hpp"

#include "system/Sys.hpp"
#import <CoreFoundation/CoreFoundation.h>
#import <Cocoa/Cocoa.h>
#include "base/Logger.hpp"
#include <sys/sysctl.h>

namespace cal::platform {

    void logInfo() {
        @autoreleasepool{
          NSOperatingSystemVersion version =
              [NSProcessInfo processInfo].operatingSystemVersion;
          NSInteger majorVersion = version.majorVersion;
          NSInteger minorVersion = version.minorVersion;
          NSInteger patchVersion = version.patchVersion;

          LogInfo("Current OS Version: ", (long)majorVersion, ".", (long)minorVersion, ".", (long)patchVersion);

          int pageSize = getpagesize();
          LogInfo("All Page Size: ", pageSize);

          int processorCount = [NSProcessInfo processInfo].activeProcessorCount;
          LogInfo("Processor Using: ", processorCount);

          int allocationGranularity = 0;
          size_t size = sizeof(allocationGranularity);
          sysctlbyname("hw.memsize", &allocationGranularity, &size, nullptr, 0);
          LogInfo("Allocation Granularity: ", allocationGranularity);
        }
    }

    void messageBox(const char* text, MsgBoxOption opt) {

        @autoreleasepool {
            MsgBox_ r = (MsgBox_)opt;
        
            NSAlertStyle style;
            switch(r) {
                case Ok: style = NSAlertStyle::NSAlertStyleInformational; break;
                case Warn: style = NSAlertStyle::NSAlertStyleWarning; break;
                case Error: style = NSAlertStyle::NSAlertStyleCritical; break;
            }

            NSAlert *alert = [[NSAlert alloc] init];
            [alert setMessageText:[NSString stringWithUTF8String:text]];
            [alert setAlertStyle:style];
            
            // Display the alert as a modal window
            [alert runModal];
            [alert release];
        }
    }


    inline static bool dialog(Span<char> out, const char* filter_str, const char* starting_file, bool is_dir, bool is_save) 
    {
       // @autoreleasepool {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        
        // Configure the panel
        [panel setCanChooseFiles:!is_dir];
        [panel setCanChooseDirectories:is_dir];
        [panel setAllowsMultipleSelection:NO];
        [panel setAllowedFileTypes:[NSArray arrayWithObjects:[NSString stringWithUTF8String:filter_str], nil]];

        if (strlen(starting_file) > 0) {
            [panel setDirectory:[NSString stringWithUTF8String:starting_file]];
        }

        NSInteger result;

        if (is_save) {
            // Save dialog
            result = [panel runModal];
        } else {
            // Open dialog
            result = [panel runModal];
        }

        if (result == NSModalResponseOK) {
            NSURL *selectedURL = [panel URL];
            auto aout = [[selectedURL path] UTF8String];
            string::copyString(out, aout);

            [panel release];
            return true;
        } 

        [panel release];
        return false;
       // }
    }


    bool getSaveFilename(Span<char> out, const char* filter, const char* default_extension) {
	    return dialog(out, filter, "", false, true);
    }


    bool getOpenFilename(Span<char> out, const char* filter_str, const char* starting_file) {
	    return dialog(out, filter_str, starting_file, false, false);
    }


    bool getOpenDirectory(Span<char> output, const char* starting_dir) {
	    return dialog(output, nullptr, starting_dir, true, false);
    }


}

#endif