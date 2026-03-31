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
typedef __builtin_va_list va_list_scalar;
typedef int __attribute__((vector_size(16))) vector_int;

/* TYPE_STRING: String typedef */
typedef const char* string_type;

/* TYPE_POINTER: Various pointer types */
typedef scalar_int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int multi_dim_array[5][10];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_type)(int, void*);
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(struct forward_declared*, va_list_scalar);

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int field1;
    char field2;
    double field3;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    double z __attribute__((aligned(8)));
} __attribute__((designated_init));

struct __attribute__((aligned(32))) aligned_struct {
    long long data[4];
    char padding;
};

/* TYPE_UNION: Various unions */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union complex_union {
    struct {
        int type;
        union {
            int int_val;
            double dbl_val;
            void* ptr_val;
        } data;
    } tagged;
    char raw_data[16];
};

/* Recursive and mutually recursive types */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to self */
    struct recursive_node* prev;
};

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

/* Struct with incomplete array (flexible array member) */
struct flex_array {
    size_t length;
    int data[];  /* TYPE_ARRAY (incomplete) */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int hidden_data;
    struct forward_declared* link;
};

/* Forward declared struct definition */
struct forward_declared {
    int magic;
    struct opaque* back_link;
    callback_type handler;
};

/* Nested complex type */
struct container {
    struct regular_struct regular;
    union complex_union variant;
    fixed_array numbers;
    incomplete_array* dynamic;  /* Pointer to incomplete array */
    complex_callback processor;
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* C++ specific structures for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        virtual ~cpp_class() {}
        virtual void method() = 0;
    };
    
    struct cpp_wrapper {
        cpp_class* obj;
        int refcount;
    };
}
#endif

/* Global callback function type */
typedef void (*global_handler)(struct container*, union simple_union);

#endif /* VARIED_TYPES_H */
