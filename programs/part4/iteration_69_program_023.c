/* gc_types.h - Common GC-tracked type definitions */
#ifndef GC_TYPES_H
#define GC_TYPES_H

/* Force GCC to process these types with gengtype */
#pragma GCC GCC gengtype
#pragma GCC visibility push(default)

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, float);

/* TYPE_POINTER: Pointer to GC types */
typedef gc_int_t* __attribute__((user("GC"))) gc_int_ptr_t;
typedef struct gc_base_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];
typedef gc_string_t __attribute__((user("GC"))) gc_string_array_t[5];

/* Forward declarations for struct/union types */
struct __attribute__((user("GC"))) gc_base_struct;
union __attribute__((user("GC"))) gc_base_union;

/* Complex nested type to force deep analysis */
struct __attribute__((user("GC"))) gc_container {
    gc_int_t scalar;
    gc_string_t str;
    gc_callback_t callback;
    gc_int_array_t array;
    struct gc_base_struct* nested;
};

#pragma GCC visibility pop
#endif /* GC_TYPES_H */
