#pragma once

#ifdef __WIN32
#define cb_winnt 1
#endif

#if defined (__MACH__) || defined (__linux__)
#define cb_api __attribute__((visibility("default")))
#elif definde(__WIN32)
#define cb_api __declspec(dllexport)
#else
#define cb_api
#endif

#ifdef cb_dyn_lib
#if defined (__MACH__) || defined (__linux__)
#define cb_lib_export __attribute__((visibility("default")))
#define cb_lib_import
#elif defined(__WIN32)
#define cb_lib_export __declspec(dllexport)
#define cb_lib_import __declspec(dllimport)
#endif
#else
#if defined (__MACH__) || defined (__linux__)
#define cb_lib_export
#define cb_lib_import
#elif defined(__WIN32)
#define cb_lib_export __declspec(dllexport)
#define cb_lib_import __declspec(import)
#endif
#endif

#define CHECK_EMPTY(POINTER, MSG)                                              \
  if (POINTER != nullptr) {                                                    \
    LogError("A Pointer Is Not NULL, Message: {0}", MSG);                        \
    throw std::system_error(std::error_code(), MSG);                           \
  }
