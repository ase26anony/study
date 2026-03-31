/* gc_types.h - Common GC-tracked type definitions */
#ifndef GC_TYPES_H
#define GC_TYPES_H

/* Force GCC to process these types with gengtype */
#pragma GCC gengtype
#pragma GCC visibility push(default)

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String pointer type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int);

/* TYPE_POINTER: Pointer to various types */
typedef gc_int_t* __attribute__((user("GC"))) gc_int_ptr_t;
typedef struct gc_base_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];
typedef gc_string_t __attribute__((user("GC"))) gc_string_array_t[5];

/* Forward declarations for cross-references */
struct gc_base_struct;
union gc_base_union;

/* Ensure types are retained and used */
#define GC_RETAIN __attribute__((used, retain, user("GC")))

#pragma GCC visibility pop
#endif /* GC_TYPES_H */
