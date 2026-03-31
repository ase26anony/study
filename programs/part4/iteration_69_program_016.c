#ifndef GENG_TYPE_TEST_TYPES_H
#define GENG_TYPE_TEST_TYPES_H

/* Force gengtype processing with GC attributes */
#define GC_TYPE __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* TYPE_STRING: String pointer type */
typedef char* GC_TYPE string_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_TYPE callback_t)(int);

/* Forward declarations for structs and unions */
struct GC_TYPE forward_struct;
union GC_TYPE forward_union;

/* TYPE_ARRAY: Array types */
typedef int GC_TYPE int_array_t[10];
typedef struct forward_struct* GC_TYPE struct_ptr_array_t[5];

#endif /* GENG_TYPE_TEST_TYPES_H */
