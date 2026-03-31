#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct opaque* opaque_ptr_t;

/* ========== TYPE_SCALAR Definitions ========== */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef _Bool scalar_bool_t;
typedef __complex__ double complex_scalar_t;
typedef int __attribute__((vector_size(16))) vector_scalar_t;
typedef __builtin_va_list va_list_scalar_t;

/* ========== TYPE_STRING ========== */
typedef const char* string_type_t;
typedef char* mutable_string_t;

/* ========== TYPE_STRUCT with various attributes ========== */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with GNU attributes */
struct __attribute__((packed, aligned(4))) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((designated_init));

struct __attribute__((aligned(32))) overaligned_struct {
    long long data[4];
};

/* Struct with incomplete array (TYPE_ARRAY) */
struct flex_array_struct {
    int count;
    int data[];  /* Incomplete array */
};

/* Recursive struct (TYPE_POINTER within TYPE_STRUCT) */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Pointer to same type */
    struct forward_declared_struct* fwd_ref;  /* Pointer to undefined */
};

/* Now define the previously forward-declared struct */
struct forward_declared_struct {
    int id;
    struct recursive_node* node;
};

/* Complete the opaque struct definition */
struct opaque {
    void* secret_data;
    int magic_number;
};

/* ========== TYPE_UNION ========== */
union variant_data {
    int as_int;
    double as_double;
    void* as_pointer;
    struct basic_struct as_struct;
};

/* Tagged union */
struct tagged_union_container {
    int tag;
    union {
        int i;
        float f;
        struct basic_struct s;
    } data __attribute__((aligned(8)));
};

/* ========== TYPE_POINTER variations ========== */
typedef int* int_ptr_t;
typedef struct basic_struct* struct_ptr_t;
typedef union variant_data* union_ptr_t;
typedef void (*generic_func_ptr_t)(void);

/* Pointer to array */
typedef int (*array_ptr_t)[10];

/* Pointer to pointer */
typedef int** double_ptr_t;

/* ========== TYPE_ARRAY variations ========== */
typedef int fixed_array_t[100];
typedef int incomplete_array_t[];
typedef struct basic_struct struct_array_t[5];
typedef int* pointer_array_t[8];

/* Multi-dimensional arrays */
typedef int matrix_t[3][4];
typedef struct recursive_node* node_ptr_array_t[20];

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*binary_op_t)(int, int);
typedef void (*event_handler_t)(struct basic_struct*, void* user_data);
typedef struct recursive_node* (*node_factory_t)(int value);
typedef void (*varargs_callback_t)(int count, ...);

/* Callback that takes another callback */
typedef void (*callback_wrapper_t)(binary_op_t op, int a, int b);

/* ========== Complex nested type ========== */
struct complex_container {
    /* Mix of many types */
    scalar_int_t scalar;
    string_type_t name;
    struct basic_struct inner_struct;
    union variant_data variant;
    fixed_array_t numbers;
    struct recursive_node* node_list;
    event_handler_t handler;
    int (*method_matrix[2][2])(int, int);  /* 2D array of function pointers */
};

/* ========== C++ specific structures for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should generate TYPE_LANG_STRUCT in C++ mode */
    struct cpp_like_struct {
        int x;
        double y;
        virtual void method() {}  /* Makes it C++-like */
    } __attribute__((transaction_safe));
    
    /* Another with GCC attributes that might trigger lang struct */
    struct __attribute__((may_alias)) aliasing_struct {
        char data[16];
    };
}
#endif

/* Transaction-safe struct (might be TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int balance;
    struct transaction_struct* next;
};

/* ========== Function declarations using the types ========== */
void process_struct(struct basic_struct* s);
int sum_array(const int arr[], size_t count);
struct recursive_node* create_node_chain(int count);
void register_callback(event_handler_t handler);

#endif /* VARIED_TYPES_H */
