/* gengtype_test.h - Common type definitions for triggering gengtype logic */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types with GC system */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

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
typedef struct gc_base_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];
typedef float __attribute__((user("GC"))) gc_float_array_t[5][5];

/* Forward declarations for cross-references */
struct gc_base_struct;
union gc_base_union;

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    gc_callback_t callback;
    gc_int_array_t scores;
};

/* TYPE_USER_STRUCT: Another structure type */
struct __attribute__((user("GC"))) gc_user_struct {
    struct gc_base_struct* base;
    gc_int_ptr_t data;
    int __attribute__((user("GC"))) extra_field;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_base_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* as_struct;
};

/* TYPE_LANG_STRUCT: Complex nested structure (simulating language-specific) */
struct __attribute__((user("GC"))) gc_lang_struct {
    union gc_base_union data;
    struct {
        gc_int_t x __attribute__((user("GC")));
        gc_int_t y __attribute__((user("GC")));
    } point;
    gc_callback_t handlers[3];
};

/* Complex nested type with all categories */
struct __attribute__((user("GC"))) gc_complex_type {
    /* Scalar members */
    gc_int_t count;
    gc_float_t ratio;
    gc_color_t color;
    
    /* String member */
    gc_string_t description;
    
    /* Struct member */
    struct gc_base_struct base;
    
    /* Union member */
    union gc_base_union variant;
    
    /* Pointer members */
    gc_int_ptr_t int_ptr;
    struct gc_user_struct* user_ptr;
    
    /* Array members */
    gc_int_array_t values;
    struct gc_base_struct* ptr_array[5];
    
    /* Callback member */
    gc_callback_t notify;
    
    /* Nested anonymous struct (simulating lang struct) */
    struct {
        gc_int_t flags __attribute__((user("GC")));
        gc_string_t tag __attribute__((user("GC")));
    } metadata;
};

/* Force retention of type information */
#define GC_RETAIN __attribute__((used, retain, user("GC")))

#pragma GCC diagnostic pop

#endif /* GENGYPE_TEST_H */
