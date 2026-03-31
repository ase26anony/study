#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar typedefs */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef __builtin_va_list va_list_scalar;

/* TYPE_STRING: String typedef */
typedef const char* string_type;

/* TYPE_POINTER: Various pointer types */
typedef scalar_int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int multi_dim_array[5][7];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(struct forward_declared*, void*);
typedef int (*vararg_callback)(int, ...);

/* TYPE_STRUCT: Regular struct definitions */
struct simple_struct {
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

struct __attribute__((designated_init)) designated_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Union definitions */
union data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* Complex nested type definitions */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Self-referential pointer */
    struct recursive_node* prev;
};

struct container {
    union data_union data;
    struct simple_struct simple;
    fixed_array numbers;
    binary_op operation;
};

/* Struct with incomplete array at end (flexible array member) */
struct flex_array {
    int count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int hidden;
    void* secret;
};

/* Forward declared struct definition */
struct forward_declared {
    callback_t notify;
    struct opaque* related;
};

/* Nested anonymous struct/union */
struct outer {
    int tag;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
        } circle;
    } shape;
};

/* Array of function pointers */
typedef callback_t callback_array[5];

/* Complex type graph */
typedef struct graph_node* graph_ptr;
struct graph_node {
    int id;
    graph_ptr edges[10];
    struct container* metadata;
};

/* C++ specific section for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        virtual void method() = 0;
        __attribute__((transaction_safe)) void safe_method();
    };
    
    struct __attribute__((transaction_safe)) transaction_struct {
        int value;
        void update() __attribute__((transaction_safe));
    };
}
#endif

/* More GNU extensions */
struct __attribute__((scalar_storage_order("big-endian"))) big_endian_struct {
    int a;
    short b;
};

typedef __attribute__((aligned(32))) struct {
    double x, y, z;
} aligned_point;

/* Function that uses many types */
void process_types(struct recursive_node* list, 
                   union data_union* data,
                   callback_t cb,
                   struct flex_array* flex);

#endif /* VARIED_TYPES_H */
