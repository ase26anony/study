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

/* GNU extensions for scalar types */
typedef __vector_size__(16) int vector_int16;
typedef __vector_size__(8) float vector_float8;
typedef __builtin_va_list va_list_t;

/* TYPE_STRING: String typedef */
typedef const char* string_t;

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef const void* const_void_ptr_t;
typedef struct opaque* opaque_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int int_array_10[10];
typedef float float_matrix[3][3];
typedef char string_array[5][20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* user_data);
typedef int (*va_func_t)(int, ...);

/* Incomplete array type for flexible array member */
struct flex_array {
    int count;
    int data[];  /* TYPE_ARRAY (incomplete) */
};

/* TYPE_STRUCT with attributes (TYPE_USER_STRUCT) */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

struct __attribute__((designated_init)) designated_struct {
    int id;
    float value;
    char name[32];
};

/* TYPE_UNION */
union data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* Recursive structure (self-referential pointer) */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to same struct */
    struct recursive_node* prev;
};

/* Complex type graph */
struct graph_node {
    int id;
    struct graph_edge** edges;  /* Pointer to array of pointers */
    int edge_count;
};

struct graph_edge {
    struct graph_node* from;
    struct graph_node* to;
    int weight;
};

/* Union containing array of pointers */
union container {
    struct graph_node* nodes[10];
    struct graph_edge* edges[20];
    void* ptr_array[30];
};

/* Callback that uses struct pointer */
typedef void (*node_visitor_t)(struct graph_node* node, void* context);

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int magic;
    void* data;
    struct forward_declared* link;
};

/* Forward declared struct definition */
struct forward_declared {
    int value;
    struct opaque* back_link;
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* C++ specific structures (for TYPE_LANG_STRUCT when compiled as C++) */
#ifdef __cplusplus
extern "C++" {
    struct cpp_struct {
        int x, y;
        virtual void method() = 0;
    };
    
    class cpp_class {
    private:
        int private_data;
    public:
        virtual ~cpp_class() {}
        virtual void operation() = 0;
    };
}
#endif

/* Global callback function type */
typedef int (*global_handler_t)(struct packed_struct*, union data_union*, va_list_t);

#endif /* VARIED_TYPES_H */
