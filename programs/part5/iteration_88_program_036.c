#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* Forward declarations to create TYPE_UNDEFINED initially */
struct opaque;
struct forward_declared;
union forward_union;

/* TYPE_SCALAR: Various scalar typedefs */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;
typedef __builtin_va_list va_list_scalar;

/* TYPE_STRING */
typedef const char* string_type;
typedef char* mutable_string;

/* TYPE_STRUCT with various attributes */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with GNU attributes */
struct __attribute__((packed, aligned(4))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((designated_init));

struct __attribute__((aligned(32))) aligned_struct {
    long long data[4];
};

/* TYPE_UNION */
union basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* TYPE_POINTER */
typedef int* int_ptr;
typedef struct basic_struct* struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char* cv_ptr;

/* TYPE_ARRAY */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_length_array[0];
typedef int multidimensional_array[5][7];

/* TYPE_CALLBACK (function pointer types) */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(void* user_data, int event_id);
typedef char* (*string_formatter)(const char* fmt, ...);

/* Recursive and mutually recursive types */
struct recursive_node {
    int data;
    struct recursive_node* next;  /* Pointer to own type */
    struct recursive_node* prev;
};

struct graph_node_a;
struct graph_node_b;

struct graph_node_a {
    int id;
    struct graph_node_b** connections;  /* Array of pointers to another struct */
    int num_connections;
};

struct graph_node_b {
    int id;
    struct graph_node_a* primary_link;
    union {
        struct graph_node_a* a_link;
        struct graph_node_b* b_link;
    } alt;
};

/* Opaque pointer type */
struct opaque {
    void* hidden_data;
    int (*process)(struct opaque* self, int input);
};

/* Struct with flexible array member (incomplete array) */
struct flexible_array {
    size_t count;
    int data[];  /* TYPE_ARRAY classification */
};

/* Now define the forward declared structs */
struct forward_declared {
    int value;
    struct forward_declared* next;
};

union forward_union {
    struct forward_declared* fd;
    struct basic_struct* bs;
};

/* Complex nested type */
struct container {
    struct basic_struct embedded;
    union basic_union choice;
    fixed_array numbers;
    binary_op operation;
    struct flexible_array* flex;
    struct opaque* opaque_ptr;
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* C++ specific structures (for TYPE_LANG_STRUCT when compiled as C++) */
#ifdef __cplusplus
extern "C" {
#endif

struct cpp_compatible {
    int value;
    void (*method)(struct cpp_compatible*);
};

#ifdef __cplusplus
}
#endif

/* Global callback function type */
typedef void (*global_handler)(struct container*, va_list_scalar);

#endif /* VARIED_TYPES_H */
