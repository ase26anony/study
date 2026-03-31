/* gengtype_test.h - Common type definitions with GC attributes */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force gengtype processing */
#pragma GCC gengtype
#pragma GCC GCC

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, float);

/* TYPE_POINTER: Pointer to various types */
typedef gc_int_t* __attribute__((user("GC"))) gc_int_ptr_t;
typedef struct gc_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];
typedef float __attribute__((user("GC"))) gc_float_array_t[5][5];

/* Forward declarations for cross-references */
struct gc_struct;
union gc_union;
struct gc_nested;

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_struct {
    gc_int_t id;
    gc_string_t name;
    gc_int_ptr_t next;
    gc_float_array_t matrix;
    gc_callback_t callback;
};

/* TYPE_USER_STRUCT: Another structure type */
struct __attribute__((user("GC"))) gc_user_struct {
    struct gc_struct* base;
    union gc_union* data;
    int __attribute__((user("GC"))) extra_field;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_struct* as_struct;
};

/* Complex nested type for deeper analysis */
struct __attribute__((user("GC"))) gc_nested {
    struct gc_struct inner;
    union gc_union variant;
    gc_int_array_t numbers;
    struct gc_nested* __attribute__((user("GC"))) recursive;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
struct __attribute__((user("GC"))) gc_lang_struct {
    void* __attribute__((user("GC"))) lang_data;
    int lang_tag;
};

/* Alias types to force additional type enumeration */
typedef struct gc_struct __attribute__((alias("gc_struct"))) gc_struct_alias_t;
typedef union gc_union __attribute__((weak)) gc_union_weak_t;

/* Global variables to prevent elimination */
extern struct gc_struct __attribute__((used, retain)) global_gc_struct;
extern union gc_union __attribute__((used, retain)) global_gc_union;
extern gc_int_array_t __attribute__((used, retain)) global_int_array;
extern gc_callback_t __attribute__((used, retain)) global_callback;

/* Function to force type usage */
void __attribute__((user("GC"))) use_types(void);

#endif /* GENGYPE_TEST_H */
