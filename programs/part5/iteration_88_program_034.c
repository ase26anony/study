#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations that will be TYPE_UNDEFINED initially */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr_t;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar typedefs */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) int_vector;
typedef float __attribute__((vector_size(32))) float_vector;

/* Builtin types */
typedef __builtin_va_list va_list_t;

/* ==================== TYPE_STRING ==================== */
typedef const char *cstring_t;
typedef char *mutable_string_t;

/* ==================== STRUCTS/UNIONS ==================== */
/* Regular struct (TYPE_STRUCT) */
struct point {
    int x;
    int y;
};

/* Packed struct (TYPE_USER_STRUCT) */
struct __attribute__((packed)) packed_data {
    byte_t id;
    int32_t value;
    char name[8];
};

/* Aligned struct (TYPE_USER_STRUCT) */
struct __attribute__((aligned(64))) aligned_data {
    double data[8];
    int counter;
};

/* Struct with designated initializer attribute */
struct __attribute__((designated_init)) config {
    int mode;
    float threshold;
    const char *name;
};

/* ==================== TYPE_UNION ==================== */
union variant {
    int32_t as_int;
    float32_t as_float;
    void *as_ptr;
    char as_bytes[4];
};

/* Tagged union */
struct tagged_union {
    int tag;
    union {
        int int_val;
        float float_val;
        void *ptr_val;
    } data;
};

/* ==================== TYPE_POINTER ==================== */
typedef int *int_ptr_t;
typedef const struct point *const_point_ptr_t;
typedef void (*generic_callback_t)(void);

/* ==================== TYPE_ARRAY ==================== */
typedef int fixed_array_t[10];
typedef int matrix_t[5][5];
extern int incomplete_array[];

/* Struct with flexible array member */
struct flex_array {
    size_t length;
    int data[];  /* Incomplete array */
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*event_handler_t)(void *context, int event_id);
typedef struct point (*point_generator_t)(int seed);

/* Complex callback with struct parameter */
typedef void (*struct_processor_t)(const struct point *p, void *userdata);

/* ==================== RECURSIVE/INTERDEPENDENT TYPES ==================== */
/* Self-referential struct */
struct tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
};

/* Mutually recursive types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b *partner;
};

struct type_b {
    int id;
    struct type_a *owner;
    struct type_a *array_of_a[3];
};

/* ==================== OPAQUE TYPE DEFINITION ==================== */
/* Now define the previously opaque struct */
struct opaque {
    int magic;
    void *data;
    struct opaque *next;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
struct container {
    struct point points[4];
    union variant variants[2];
    int_ptr_t *pointer_array;  /* Pointer to pointer */
    binary_op_t operations[3];
    struct flex_array *flex;
};

/* ==================== C++ SPECIFIC (for TYPE_LANG_STRUCT) ==================== */
#ifdef __cplusplus
extern "C++" {
    /* This should generate TYPE_LANG_STRUCT in C++ mode */
    struct cpp_like_struct {
        int value;
        void method() {}
    };
    
    /* Transaction-safe struct */
    struct __attribute__((transaction_safe)) transaction_struct {
        int data;
        void update() {}
    };
}
#endif

/* ==================== FUNCTION DECLARATIONS ==================== */
/* For other.c */
void process_types(struct tree_node *root, struct_processor_t processor);
int compute_sum(const fixed_array_t arr, int size);

#endif /* VARIED_TYPES_H */
