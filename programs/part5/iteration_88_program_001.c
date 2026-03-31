#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr;

/* TYPE_SCALAR: Various scalar types */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef enum { RED, GREEN, BLUE } color_t;

/* GCC extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) v4si;
typedef float __attribute__((vector_size(32))) v8sf;

/* TYPE_STRING: String types */
typedef const char *cstring_t;
typedef char *mutable_string_t;

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
    int counter;
};

struct __attribute__((designated_init)) designated_init_struct {
    int x;
    int y;
    int z;
};

/* TYPE_STRUCT: Regular structs */
struct point {
    int x;
    int y;
    int z;
};

struct data_record {
    int id;
    char name[32];
    struct point location;
    float values[16];
};

/* TYPE_UNION: Various unions */
union variant {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
};

union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
    double real;
};

/* TYPE_POINTER: Pointer types */
typedef int *int_ptr;
typedef struct point *point_ptr;
typedef const struct data_record *const_data_ptr;
typedef void (*generic_callback)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct point point_array_5[5];
typedef float matrix_3x3[3][3];

/* Incomplete array (TYPE_ARRAY) */
struct flexible_array {
    int count;
    int data[];  /* Incomplete array */
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*event_handler_t)(int event_id, void *user_data);
typedef char *(*string_formatter_t)(const char *fmt, ...);

/* Complex callback with struct parameter */
typedef void (*data_processor_t)(struct data_record *rec, union variant *var);

/* Recursive types */
struct tree_node {
    int value;
    struct tree_node *left;   /* Pointer to same type */
    struct tree_node *right;  /* Pointer to same type */
};

/* Mutual recursion */
struct list_node;
struct list_header {
    struct list_node *first;
    struct list_node *last;
};

struct list_node {
    int data;
    struct list_node *next;
    struct list_node *prev;
    struct list_header *header;
};

/* Opaque pointer type (initially TYPE_UNDEFINED) */
struct opaque {
    /* Definition provided later */
    void *internal_data;
    int (*process)(struct opaque *self);
};

/* Now define the forward declared struct */
struct forward_declared {
    int magic_number;
    struct opaque *related;
};

/* Builtin types */
typedef __builtin_va_list va_list_t;

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_data {
    int transaction_id;
    void *payload;
    size_t size;
};

/* C++ mode specific (for TYPE_LANG_STRUCT) */
#ifdef __cplusplus
extern "C++" {
    struct cpp_compatible {
        int x;
        double y;
        virtual void method() = 0;
    };
}
#endif

/* Complex nested type */
struct container {
    struct {
        int tag;
        union {
            int int_val;
            float float_val;
            char *string_val;
        } data;
    } variant;
    
    struct tree_node *tree_root;
    point_array_5 points;
    comparator_t compare_func;
    struct flexible_array *flex_array;
};

/* Callback that uses multiple types */
typedef void (*complex_callback_t)(
    struct container *cont,
    union variant *var,
    int_array_10 *arr,
    data_processor_t processor
);

#endif /* VARIED_TYPES_H */
