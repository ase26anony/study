/* gengtype_test.h - Common header with GC-tracked type definitions */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force gengtype processing with GCC attributes */
#define GC_TYPE __attribute__((user("GC")))
#define GC_USED __attribute__((used, retain))

/* Scalar types (TYPE_SCALAR) */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* String type (TYPE_STRING) */
typedef char* GC_TYPE string_ptr_t;

/* Callback type (TYPE_CALLBACK) */
typedef void (*GC_TYPE callback_t)(int, void*);

/* Pointer types (TYPE_POINTER) */
typedef scalar_int_t* GC_TYPE int_ptr_t;
typedef void* GC_TYPE generic_ptr_t;

/* Array type (TYPE_ARRAY) */
typedef int GC_TYPE int_array_t[10];

/* Forward declarations for structs/unions */
struct GC_TYPE base_struct;
union GC_TYPE base_union;

/* Complex nested type definitions */
struct GC_TYPE nested_member {
    int id;
    struct base_struct* next;
    callback_t handler;
};

#endif /* GENGYPE_TEST_H */
