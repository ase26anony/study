/* gengtype_test.h - Common type definitions for GC testing */
#ifndef GENG_TYPE_TEST_H
#define GENG_TYPE_TEST_H

/* Force gengtype processing with GCC attributes */
#define GC_TYPE __attribute__((user("GC")))
#define GC_USED __attribute__((used, retain))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* TYPE_STRING: String pointer type */
typedef char* GC_TYPE string_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_TYPE callback_t)(int, float);

/* TYPE_POINTER: Various pointer types */
typedef scalar_int_t* GC_TYPE int_ptr_t;
typedef void* GC_TYPE generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int GC_TYPE int_array_t[10];
typedef scalar_int_t GC_TYPE scalar_array_t[5];

/* Forward declarations for struct/union types */
struct GC_TYPE forward_struct;
union GC_TYPE forward_union;

#endif /* GENG_TYPE_TEST_H */
