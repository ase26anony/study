#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr;

/* TYPE_SCALAR: Various scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __builtin_va_list va_list_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;

/* TYPE_STRING: String types */
typedef const char *string_type;
typedef char *mutable_string;

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Various unions */
union basic_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef int *int_ptr;
typedef struct regular_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int multi_dim_array[5][10];
extern int external_array[];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void *context, int event_id);
typedef int (*va_func)(int, ...);

/* Recursive and mutually recursive types */
struct recursive_node {
    int value;
    struct recursive_node *next;  /* TYPE_POINTER to self */
};

struct graph_node {
    int id;
    struct graph_edge *edges;  /* Forward reference */
};

struct graph_edge {
    struct graph_node *from;
    struct graph_node *to;
    int weight;
};

/* Union with array of pointers */
union complex_union {
    struct graph_node *node_array[4];
    void *ptr_array[8];
    long long as_ll;
};

/* Struct with flexible array member */
struct flex_array {
    size_t count;
    int data[];  /* Incomplete array */
};

/* Opaque pointer type (initially TYPE_UNDEFINED) */
struct opaque {
    void *hidden_data;
    int secret;
};

/* Language-specific extensions */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific {
        int cpp_field;
        virtual void method() = 0;
    } __attribute__((transaction_safe));
}
#else
/* For C, use transaction attribute directly */
struct __attribute__((transaction_safe)) transaction_struct {
    int tx_field;
    long tx_data;
};
#endif

/* Nested type combinations */
struct container {
    struct regular_struct regular;
    union basic_union uni;
    fixed_array arr;
    binary_op callback;
    struct recursive_node *node_ptr;
    incomplete_array *flex_ptr;  /* Pointer to incomplete array */
};

/* Callback that uses our types */
typedef void (*processor_callback)(struct container *, union complex_union *);

#endif /* VARIED_TYPES_H */
