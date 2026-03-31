#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process types for garbage collection */
#define GC_TYPE __attribute__((user("GC")))

/* Scalar types (TYPE_SCALAR) */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef double GC_TYPE scalar_double_t;

/* Enum type (also TYPE_SCALAR) */
typedef enum GC_TYPE {
    STATE_A,
    STATE_B,
    STATE_C
} state_t;

/* String type (TYPE_STRING) */
typedef char* GC_TYPE string_t;

/* Callback type (TYPE_CALLBACK) */
typedef void (*GC_TYPE callback_t)(int, void*);

/* Pointer types (TYPE_POINTER) */
typedef scalar_int_t* GC_TYPE int_ptr_t;
typedef void* GC_TYPE generic_ptr_t;

/* Array type (TYPE_ARRAY) */
typedef int GC_TYPE int_array_t[10];

/* Forward declarations for struct/union types */
struct GC_TYPE base_struct;
union GC_TYPE data_union;

/* Complex nested type definitions */
struct GC_TYPE inner_struct {
    scalar_int_t id;
    string_t name;
    callback_t handler;
};

union GC_TYPE inner_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

#endif /* GENGYPE_TEST_H */
