#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar typedefs */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;

/* GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* TYPE_STRING: String typedef */
typedef const char* string_t;

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((designated_init)) designated_init_struct {
    int x;
    double y;
    char z;
};

/* TYPE_STRUCT: Regular structs */
struct point {
    int x;
    int y;
    int z;
};

/* Recursive struct (TYPE_POINTER within TYPE_STRUCT) */
struct recursive_node {
    int data;
    struct recursive_node *next;  /* Self-reference */
    struct recursive_node *prev;
};

/* TYPE_UNION */
union variant {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
};

/* TYPE_ARRAY: Various array types */
typedef int fixed_array_t[10];
typedef int matrix_t[5][5];
typedef int incomplete_array_t[];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(struct point*, void*);
typedef int (*va_func_t)(int, ...);

/* Complex callback with struct parameter */
typedef struct recursive_node* (*node_factory_t)(int, struct recursive_node*);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct point* point_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Struct with incomplete array at end (flexible array member) */
struct flex_array {
    int length;
    int data[];  /* TYPE_ARRAY */
};

/* Now define the previously forward-declared structs */
struct opaque {
    void *data;
    int type;
};

struct forward_declared {
    struct opaque *opaque_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
    int value;
};

/* Nested complex type */
struct container {
    struct point position;
    union variant data;
    fixed_array_t items;
    binary_op_t operation;  /* TYPE_CALLBACK */
    struct container *next;
};

/* Union containing array of pointers */
union pointer_union {
    struct point* points[5];
    struct recursive_node* nodes[5];
    void* generic[5];
};

/* __builtin_va_list usage */
typedef __builtin_va_list va_list_t;

/* C++ specific structures for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    struct cpp_like_struct {
        int x;
        double y;
        virtual void method() = 0;
    };
    
    class SimpleClass {
        int private_data;
    public:
        SimpleClass() : private_data(0) {}
        int get() const { return private_data; }
    };
}
#endif

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* Complex nested type with all classifications */
struct master_type {
    /* TYPE_SCALAR */
    int32_t scalar;
    
    /* TYPE_STRING */
    string_t name;
    
    /* TYPE_STRUCT */
    struct point location;
    
    /* TYPE_UNION */
    union variant value;
    
    /* TYPE_POINTER */
    struct master_type *self_ptr;
    
    /* TYPE_ARRAY */
    fixed_array_t numbers;
    
    /* TYPE_CALLBACK */
    callback_t handler;
    
    /* TYPE_USER_STRUCT */
    struct packed_struct packed;
    
    /* Flexible array member */
    int dynamic_data[];
};

#endif /* VARIED_TYPES_H */
