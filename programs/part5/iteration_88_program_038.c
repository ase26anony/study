#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar types */
typedef char my_char;
typedef int my_int;
typedef long my_long;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef void my_void;

/* GNU extensions for scalar types */
typedef __complex__ double my_complex;
typedef int __attribute__((vector_size(16))) my_vector;
typedef __builtin_va_list my_va_list;

/* TYPE_STRING: String types */
typedef const char* my_string;
typedef char* mutable_string;

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;
typedef int (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_array[0];
typedef int (*array_of_ptrs[5])(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(void* data, int value);
typedef int (*variadic_func)(int, ...);

/* TYPE_STRUCT: Regular structs */
struct point {
    int x;
    int y;
    int z;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_struct {
    int field1;
    char field2;
    double field3;
};

/* TYPE_UNION: Union types */
union data_union {
    int as_int;
    float as_float;
    char as_bytes[4];
    void* as_ptr;
};

/* Complex nested type with incomplete array */
struct flexible_array {
    int count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Recursive and mutually recursive types */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to self */
    struct forward_declared* fwd; /* TYPE_UNDEFINED initially */
};

/* Now define the forward declared struct */
struct forward_declared {
    int id;
    struct recursive_node* node;
};

/* Opaque struct definition */
struct opaque {
    int secret;
    char* name;
};

/* Union containing array of pointers */
union container {
    struct point points[3];
    int* ptr_array[5];
    binary_op funcs[2];
};

/* Struct with function pointer member */
struct processor {
    int id;
    callback_t process;
    void* user_data;
};

/* For TYPE_LANG_STRUCT - use C++ if available */
#ifdef __cplusplus
extern "C++" {
    struct cpp_like_struct {
        int value;
        virtual void method() = 0;
    } __attribute__((transaction_safe));
}
#else
/* Use transaction attribute for C */
struct __attribute__((transaction_safe)) transaction_struct {
    int atomic_value;
    void* resource;
};
#endif

/* Complex type graph */
typedef struct graph_node {
    int id;
    struct graph_node** neighbors;  /* Array of pointers */
    int neighbor_count;
} graph_node_t;

/* Callback that uses multiple types */
typedef void (*complex_callback)(struct processor*, union data_union, graph_node_t*);

#endif /* VARIED_TYPES_H */
