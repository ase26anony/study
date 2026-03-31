/* gengtype_test.h - Common header with GC-tracked type definitions */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types with gengtype */
#pragma GCC GCC
#pragma GCC gengtype

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String pointer type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, float);

/* TYPE_POINTER: Pointer to various types */
typedef gc_int_t* __attribute__((user("GC"))) gc_int_ptr_t;
typedef struct gc_base_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];
typedef gc_string_t __attribute__((user("GC"))) gc_string_array_t[5];

/* Forward declarations for cross-references */
struct gc_base_struct;
union gc_complex_union;
struct gc_nested_struct;

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    gc_callback_t callback;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct __attribute__((user("GC"), aligned(16))) gc_user_struct {
    gc_int_array_t scores;
    gc_struct_ptr_t next;
    struct gc_nested_struct* nested;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_simple_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
};

/* Complex nested union */
union __attribute__((user("GC"))) gc_complex_union {
    struct gc_base_struct base;
    struct gc_user_struct user;
    union gc_simple_union simple;
    gc_int_array_t array;
};

/* TYPE_STRUCT with nested types */
struct __attribute__((user("GC"))) gc_nested_struct {
    struct gc_base_struct base;
    union gc_complex_union data;
    gc_int_ptr_t int_ptrs[8];
    struct gc_nested_struct* __attribute__((user("GC"))) children[4];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct __attribute__((user("GC"), may_alias)) gc_lang_struct {
    void* __attribute__((user("GC"))) vtable;
    gc_int_t type_tag;
    union {
        gc_int_t int_val;
        gc_float_t float_val;
        gc_string_t string_val;
    } __attribute__((user("GC"))) value;
};

/* Create type aliases to force additional processing */
typedef struct gc_base_struct __attribute__((user("GC"), alias("gc_base_struct"))) gc_base_alias_t;
typedef union gc_complex_union __attribute__((user("GC"), weak)) gc_weak_union_t;

/* Global variables to force type instantiation */
extern struct gc_base_struct __attribute__((used, retain)) global_base;
extern struct gc_user_struct __attribute__((used, retain)) global_user;
extern union gc_complex_union __attribute__((used, retain)) global_union;
extern struct gc_nested_struct __attribute__((used, retain)) global_nested;
extern struct gc_lang_struct __attribute__((used, retain)) global_lang;

/* Function declarations */
void init_gc_types(void);
void traverse_gc_types(void);
unsigned long calculate_checksum(void);

#endif /* GENGYPE_TEST_H */
