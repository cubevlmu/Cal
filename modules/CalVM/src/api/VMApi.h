#pragma once

#include <stdint.h>
#include <stdio.h>

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

typedef struct CalVM* CalVMRef;
typedef struct calModule* CalModuleRef;
typedef struct calFunc* calFuncRef;
typedef struct calStruct* calStructRef;
typedef struct calClass* calClassRef;
typedef struct calField* calFieldRef;

typedef void(*calNativeFunc)(CalVMRef);

// VM
bool cal_init_vm(CalVMRef* state);
bool cal_stop_vm(CalVMRef* state);

// Module
bool cal_load_module(CalVMRef vm, const char* path, CalModuleRef* module);
bool cal_release_module(CalVMRef vm, CalModuleRef module);
bool cal_find_symbol_func(CalVMRef vm, const char* name, const char* sig, calFuncRef* func);
bool cal_find_symbol_struct(CalVMRef vm, const char* name, calStructRef* func);
bool cal_find_symbol_class(CalVMRef vm, const char* name, calClassRef* func);
bool cal_find_symbol_constants(CalVMRef vm, const char* name, calClassRef* func);

// Std
bool cal_load_std(CalVMRef vm);
bool cal_unload_std(CalVMRef vm);

// String utils
bool cal_get_string(CalVMRef vm, calStringRef ref, const char** str);
bool cal_set_string(CalVMRef vm, calStringRef ref, const char* str, int encode);

// Objects
bool cal_create_object(CalVMRef vm, calClassRef clazz, calObjectRef* result);
bool cal_create_struct(CalVMRef vm, calStructRef clazz, calObjectRef* result);
bool cal_delete_object(CalVMRef vm, calObjectRef result); // don't use it

// Operators
bool cal_call_func(CalVMRef vm, calFuncRef ref, void* args);
bool cal_find_class_func(CalVMRef vm, calClassRef clazz, const char* name, const char* sig, calFuncRef* func);
bool cal_find_class_field(CalVMRef vm, calClassRef clazz, const char* name, calFieldRef* func);

bool cal_set_field_value(CalVMRef vm, calFieldRef field, calObjectRef obj, void* value);
bool cal_get_field_value(CalVMRef vm, calFieldRef field, calObjectRef obj, void** value);

// VM controls
bool cal_gc(CalVMRef vm);
bool cal_get_error(CalVMRef vm, const char** error);
bool cal_dump_memory(CalVMRef vm);
bool cal_debug_dump(CalVMRef vm);
bool cal_vm_exit_requst(CalVMRef vm);
bool cal_vm_exit_code(CalVMRef vm, i32* code);

bool cal_register_native_func(CalVMRef vm, calNativeFunc func, const char* name, void* sig);
bool cal_unregister_native_func(CalVMRef vm, calNativeFunc func, const char* name, const char* sig);

// Native utils
bool cal_clear_local_ref(CalVMRef vm, void* ref);
bool cal_build_native_func_sig(CalVMRef vm, void** sig, void* sigParts);
