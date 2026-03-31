#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int;
typedef char scalar_char;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;      /* GNU extension */
typedef __complex__ float complex_float_scalar; /* GNU extension */
typedef __builtin_va_list va_list_scalar;       /* Builtin type */

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));  /* TYPE_SCALAR? */

/* ========== TYPE_STRING ========== */
typedef const char* string_type;
typedef char* mutable_string;

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;
typedef int (*func_ptr)(void);

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];
typedef int incomplete_array[];  /* Incomplete array type */
typedef int multi_dim_array[5][7];

/* ========== TYPE_CALLBACK (Function Pointers) ========== */
typedef int (*callback_type)(int, char*);
typedef void (*void_callback)(void);
typedef struct opaque* (*opaque_callback)(int, va_list_scalar);

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int a;
    char b;
    double c;
};

/* Packed struct for TYPE_USER_STRUCT */
struct packed_struct {
    int x;
    char y;
    double z;
} __attribute__((packed, aligned(16)));  /* TYPE_USER_STRUCT likely */

/* Struct with designated init attribute */
struct designated_struct {
    int field1;
    char field2;
    double field3;
} __attribute__((designated_init));

/* Struct with incomplete array as last member */
struct flexible_array_struct {
    int count;
    int data[];  /* Incomplete array */
};

/* ========== TYPE_UNION ========== */
union simple_union {
    int as_int;
    float as_float;
    char* as_string;
};

union complex_union {
    struct simple_struct s;
    union simple_union u;
    void* ptr;
    long long big_int;
};

/* ========== Recursive and Interconnected Types ========== */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Pointer to self */
    struct recursive_node* prev;
};

struct graph_node {
    int id;
    struct graph_node* neighbors[8];  /* Array of pointers */
    union complex_union data;
};

/* Now define the previously opaque struct */
struct opaque {
    int magic;
    struct graph_node* node;
    callback_type callback;
};

/* Define the forward declared struct */
struct forward_declared_struct {
    struct opaque* opaque_ptr;
    incomplete_array array_ref;  /* Reference to incomplete array type */
};

/* ========== Complex Nested Type ========== */
struct container {
    struct simple_struct simple;
    union simple_union uni;
    fixed_array arr;
    struct recursive_node* node_list;
    callback_type handlers[3];
    struct {
        int anonymous_member;
        char anonymous_char;
    } anonymous_struct;
    volatile int volatile_member;
    const char* const_string;
};

/* ========== TYPE_LANG_STRUCT (C++ specific) ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific_struct {
        int cpp_only_field;
        virtual void method() = 0;
    } __attribute__((transaction_safe));
}
#else
/* For C, use transaction attribute directly */
struct transaction_struct {
    int data;
    char* info;
} __attribute__((transaction_safe));
#endif

/* ========== Function using complex types ========== */
#ifdef __cplusplus
extern "C" {
#endif

/* Callback that uses many different types */
typedef int (*complex_callback)(
    struct container*,
    union complex_union*,
    va_list_scalar
);

/* Function pointer with array parameter */
typedef void (*array_callback)(int[], incomplete_array);

/* Struct containing function pointers */
struct callback_container {
    callback_type simple_cb;
    complex_callback complex_cb;
    array_callback array_cb;
    void_callback void_cb;
};

/* Union with function pointer */
union callback_union {
    int (*func_int)(int);
    void (*func_void)(void);
    struct opaque* (*func_opaque)(void);
};

#ifdef __cplusplus
}
#endif

#endif /* VARIED_TYPES_H */
