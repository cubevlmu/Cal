#pragma once

#include <stdint.h>
#include <stdio.h>

extern "C" {

    typedef int i32;
    typedef long long i64;
    typedef int8_t i8;
    typedef int16_t i16;

    typedef uint32_t u32;
    typedef uint64_t u64;
    typedef uint8_t u8;
    typedef uint16_t u16;

    typedef struct calObject* calObjectRef;
    typedef struct calString* calStringRef;

    typedef struct calVM* CalVMRef;
    typedef struct calModule* CalModuleRef;
    typedef struct calFunc* calFuncRef;
    typedef struct calStruct* calStructRef;
    typedef struct calClass* calClassRef;
    typedef struct calField* calFieldRef;

    typedef void(*calNativeFunc)(calVM*);

    // VM
    bool cal_init_vm(CalVMRef* state);
    bool cal_stop_vm(CalVMRef* state);

    // Module
    bool cal_load_module(calVM* vm, const char* path, CalModuleRef* module);
    bool cal_release_module(calVM* vm, CalModuleRef module);
    bool cal_find_symbol_func(calVM* vm, const char* name, const char* sig, calFuncRef* func);
    bool cal_find_symbol_struct(calVM* vm, const char* name, calStructRef* func);
    bool cal_find_symbol_class(calVM* vm, const char* name, calClassRef* func);
    bool cal_find_symbol_constants(calVM* vm, const char* name, calClassRef* func);

    // Std
    bool cal_load_std(calVM* vm);
    bool cal_unload_std(calVM* vm);

    // String utils
    bool cal_get_string(calVM* vm, calStringRef ref, const char** str);
    bool cal_set_string(calVM* vm, calStringRef ref, const char* str, int encode);

    // Objects
    bool cal_create_object(calVM* vm, calClassRef clazz, calObjectRef* result);
    bool cal_create_struct(calVM* vm, calStructRef clazz, calObjectRef* result);
    bool cal_delete_object(calVM* vm, calObjectRef result); // don't use it

    // Operators
    bool cal_call_func(calVM* vm, calFuncRef ref, void* args);
    bool cal_find_class_func(calVM* vm, calClassRef clazz, const char* name, const char* sig, calFuncRef* func);
    bool cal_find_class_field(calVM* vm, calClassRef clazz, const char* name, calFieldRef* func);

    bool cal_set_field_value(calVM* vm, calFieldRef field, calObjectRef obj, void* value);
    bool cal_get_field_value(calVM* vm, calFieldRef field, calObjectRef obj, void** value);

    // VM controls
    bool cal_gc(calVM* vm);
    bool cal_get_error(calVM* vm, const char** error);
    bool cal_dump_memory(calVM* vm);
    bool cal_debug_dump(calVM* vm);
    bool cal_vm_exit_requst(calVM* vm);
    bool cal_vm_exit_code(calVM* vm, i32* code);

    bool cal_register_native_func(calVM* vm, calNativeFunc func, const char* name, void* sig);
    bool cal_unregister_native_func(calVM* vm, calNativeFunc func, const char* name, const char* sig);

    // Native utils
    bool cal_clear_local_ref(calVM* vm, void* ref);
    bool cal_build_native_func_sig(calVM* vm, void** sig, void* sigParts);

}