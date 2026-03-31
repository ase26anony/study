#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar types and typedefs */
typedef int my_int;
typedef char my_char;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef void my_void;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* TYPE_STRING: String typedef */
typedef const char* my_string;

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Unions */
union data_union {
    int as_int;
    float as_float;
    double as_double;
    char as_char[8];
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct regular_struct* struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile int* cv_int_ptr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct regular_struct struct_array[5];
extern int incomplete_array[];  /* Incomplete array type */

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(struct regular_struct*, int);
typedef void (*varargs_callback)(int, ...);

/* Recursive and mutually recursive types for complex graphs */
struct recursive_struct {
    int data;
    struct recursive_struct* next;  /* Pointer to own type */
    struct forward_declared* fwd;   /* Pointer to forward declared */
};

/* Now define the forward declared struct */
struct forward_declared {
    int id;
    struct recursive_struct* recursive;
    union data_union data;
};

/* Struct with flexible array member (incomplete array) */
struct flex_array_struct {
    int length;
    int data[];  /* Incomplete array as last member */
};

/* Opaque pointer type */
typedef struct opaque* opaque_ptr;

/* Later definition of opaque struct */
struct opaque {
    int secret;
    void* data;
    struct opaque* next;
};

/* Complex nested type */
struct container {
    struct regular_struct regular;
    union data_union uni;
    int_array arr;
    struct recursive_struct* recursive_ptr;
    binary_op operation;
};

/* Union containing array of pointers */
union pointer_union {
    struct regular_struct* struct_ptrs[4];
    void* void_ptrs[4];
    int* int_ptrs[4];
};

/* Callback that uses multiple types */
typedef struct container* (*complex_callback)(
    struct recursive_struct*, 
    union data_union, 
    int_array
);

/* For TYPE_LANG_STRUCT - use C++ if available */
#ifdef __cplusplus
extern "C++" {
    struct cpp_struct {
        int x;
        double y;
        virtual void method() {}
    } __attribute__((transaction_safe));
}
#else
/* Alternative for C: use transaction_safe attribute */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void* ptr;
};
#endif

/* Another user struct with multiple attributes */
struct __attribute__((packed, aligned(8))) multi_attr_struct {
    char a;
    int b;
    long c;
} __attribute__((deprecated));

#endif /* VARIED_TYPES_H */
