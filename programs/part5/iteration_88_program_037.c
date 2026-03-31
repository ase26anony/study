#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct opaque *opaque_ptr_t;

/* ========== TYPE_SCALAR Definitions ========== */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef _Bool scalar_bool_t;
typedef __complex__ double complex_scalar_t;
typedef int __attribute__((vector_size(16))) vector_scalar_t;
typedef __builtin_va_list va_list_scalar_t;

/* ========== TYPE_STRING ========== */
typedef const char *string_ptr_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT (Regular structs) ========== */
struct regular_struct {
    int field1;
    char field2;
    double field3;
};

/* ========== TYPE_USER_STRUCT (with attributes) ========== */
struct __attribute__((packed, aligned(8))) packed_user_struct {
    int a;
    char b;
    double c __attribute__((aligned(16)));
};

struct __attribute__((designated_init)) designated_init_struct {
    int x;
    double y;
    char z;
};

/* ========== TYPE_UNION ========== */
union data_union {
    int as_int;
    double as_double;
    char as_char[8];
    void *as_ptr;
};

/* ========== TYPE_POINTER ========== */
typedef int *int_ptr_t;
typedef struct regular_struct *struct_ptr_t;
typedef void (*generic_func_ptr_t)(void);
typedef int (*array_of_func_ptrs_t[5])(void);

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array_t[10];
typedef int incomplete_array_t[];
typedef int zero_length_array_t[0];
typedef int multidimensional_array_t[3][4][5];

/* ========== TYPE_CALLBACK (Function pointers) ========== */
typedef int (*binary_callback_t)(int, int);
typedef void (*struct_callback_t)(struct regular_struct*);
typedef int (*variadic_callback_t)(int, ...);

/* ========== Complex Nested/Recursive Types ========== */
/* Recursive struct with pointer to self */
struct recursive_node {
    int data;
    struct recursive_node *next;
    struct recursive_node *prev;
};

/* Struct containing union with array of pointers */
struct container {
    int id;
    union {
        struct regular_struct *struct_ptrs[4];
        struct recursive_node *node_ptrs[4];
    } ptr_union;
    binary_callback_t callback;
};

/* Struct with incomplete array at end */
struct flexible_array {
    int count;
    double average;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* ========== Interconnected Type Graph ========== */
struct type_a;
struct type_b;

struct type_a {
    struct type_b *b_ptr;
    binary_callback_t processor;
    int values[5];
};

struct type_b {
    struct type_a *a_ptr;
    union data_union storage;
    struct_callback_t notifier;
};

/* ========== Opaque struct definition (after forward declaration) ========== */
struct opaque {
    int hidden_data;
    struct forward_declared_struct *link;
};

/* ========== Forward-declared struct definition ========== */
struct forward_declared_struct {
    opaque_ptr_t opaque_link;
    struct_callback_t handler;
};

/* ========== C++ specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific_struct {
        int cpp_field;
        virtual void method() {}
    } __attribute__((transaction_safe));
}
#else
/* For C, use transaction_safe attribute directly */
struct __attribute__((transaction_safe)) c_lang_struct {
    int lang_field;
    void (*lang_method)(void);
};
#endif

/* ========== Function declarations using the types ========== */
void process_types(struct container *c);
int calculate_sum(binary_callback_t cb, int a, int b);
struct flexible_array *create_flexible(int count);

#endif /* VARIED_TYPES_H */
