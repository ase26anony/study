#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar types */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef enum { RED, GREEN, BLUE } color_t;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) int_vec4;
typedef float __attribute__((vector_size(32))) float_vec8;

/* TYPE_STRING */
typedef const char* string_ptr;

/* TYPE_POINTER */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;

/* TYPE_ARRAY */
typedef int fixed_array[10];
typedef int matrix[3][3];
extern int incomplete_array[];

/* TYPE_CALLBACK (function pointers) */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef int (*va_func_t)(int count, ...);

/* TYPE_STRUCT with various attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(32))) aligned_struct {
    double data;
    int tag;
};

struct __attribute__((designated_init)) designated_struct {
    int id;
    const char* name;
    float value;
};

/* TYPE_USER_STRUCT with multiple attributes */
struct __attribute__((packed, aligned(8))) user_struct {
    unsigned char flags;
    unsigned int count;
    void* payload;
};

/* TYPE_UNION */
union data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* Complex nested types */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Self-referential pointer */
    struct recursive_node* prev;
};

struct container {
    int id;
    union data_union storage;
    struct recursive_node* node_list;
    binary_op operation;
};

/* Struct with incomplete array (flexible array member) */
struct flex_array {
    size_t length;
    int data[];  /* Incomplete array */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int secret;
    void* handle;
};

/* Forward declared struct definition */
struct forward_declared {
    struct container* cont;
    struct opaque* opq;
};

/* Array of pointers to struct */
typedef struct container* container_array[5];

/* Nested struct with function pointer */
struct processor {
    int (*transform)(struct processor*, void*);
    callback_t on_complete;
    union data_union state;
};

/* TYPE_LANG_STRUCT candidates */
#ifdef __cplusplus
extern "C++" {
    struct cpp_struct {
        int value;
        virtual ~cpp_struct() {}
    };
    
    class cpp_class {
    public:
        virtual void method() = 0;
        int data;
    };
}
#else
/* GCC transaction-safe attribute (may create lang struct) */
struct __attribute__((transaction_safe)) transaction_struct {
    int balance;
    void (*commit)(struct transaction_struct*);
};
#endif

/* Builtin types */
typedef __builtin_va_list va_list_t;

/* Complex type graph */
struct type_graph {
    struct type_graph* left;
    struct type_graph* right;
    union {
        int leaf_value;
        struct type_graph* child;
    } u;
    void (*visit)(struct type_graph*);
};

#endif /* VARIED_TYPES_H */
