/* gengtype_test.h - Common type definitions for gengtype coverage test */

#ifndef GENG_TYPE_TEST_H
#define GENG_TYPE_TEST_H

/* Force GCC to process these types with GC system */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

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

/* Forward declarations for recursive structures */
struct gc_base_struct;
union gc_complex_union;

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    gc_int_ptr_t next;
};

/* TYPE_USER_STRUCT: Another structure type */
struct __attribute__((user("GC"))) gc_user_struct {
    gc_int_t data[4];
    gc_callback_t handler;
    struct gc_base_struct* base;
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
    gc_int_array_t numbers;
    union gc_simple_union simple;
};

/* TYPE_LANG_STRUCT: Structure that might be treated specially */
struct __attribute__((user("GC"))) gc_lang_struct {
    gc_int_t tag;
    union {
        gc_int_t int_val;
        gc_float_t float_val;
        gc_string_t str_val;
        struct gc_base_struct* struct_ptr;
    } data;
};

/* Alias types to force additional type processing */
typedef struct gc_base_struct __attribute__((user("GC"), alias("gc_base_struct"))) gc_base_alias_t;
typedef union gc_simple_union __attribute__((user("GC"), weak)) gc_weak_union_t;

/* Global variables to prevent optimization */
extern struct gc_base_struct __attribute__((used, retain)) global_base;
extern struct gc_user_struct __attribute__((used, retain)) global_user;
extern union gc_complex_union __attribute__((used, retain)) global_complex;
extern gc_callback_t __attribute__((used, retain)) global_callback;

/* Function to force type usage */
void use_types(void);

#pragma GCC diagnostic pop

#endif /* GENG_TYPE_TEST_H */
