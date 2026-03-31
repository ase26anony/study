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
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types */
typedef const char *string_ptr;
typedef char *mutable_string;

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(64))) aligned_struct {
    double data;
    int metadata;
};

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    char field2;
    long field3;
};

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int id;
    char name[32];
    float value;
};

/* TYPE_UNION: Unions */
union data_union {
    int as_int;
    float as_float;
    char as_char[4];
    void *as_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef struct regular_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Arrays */
typedef int fixed_array[10];
typedef int matrix[5][5];
extern int incomplete_array[];
typedef struct regular_struct struct_array[3];

/* TYPE_CALLBACK: Function pointers */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(int, void *);
typedef char *(*string_processor)(const char *);

/* Complex nested types */
struct recursive_struct {
    int data;
    struct recursive_struct *next;  /* Self-referential pointer */
};

struct container {
    struct regular_struct regular;
    union data_union variant;
    fixed_array numbers;
    struct recursive_struct *recursive_head;
};

/* Incomplete array at end of struct */
struct flexible_array {
    int count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Opaque pointer type */
struct opaque {
    void *hidden_data;
    int type_id;
};

/* Complex type graph */
struct node {
    int value;
    struct edge *edges;
};

struct edge {
    struct node *from;
    struct node *to;
    int weight;
};

/* Union with array of pointers */
union pointer_union {
    struct node *node_ptrs[10];
    struct edge *edge_ptrs[20];
    void *generic_ptrs[5];
};

/* Callback with complex signature */
typedef void (*complex_callback)(
    struct container *,
    union data_union *,
    binary_op,
    va_list
);

/* GCC extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* Now define the forward declared struct */
struct forward_declared {
    int magic;
    struct opaque *opaque_ptr;
};

/* Function declarations using the types */
void process_container(struct container *c);
int sum_array(const fixed_array arr);
union data_union create_union(int type);

#endif /* VARIED_TYPES_H */
