/* varied_types.h - Comprehensive type definitions for gengtype testing */

#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete *incomplete_ptr_t;

/* ========== TYPE_SCALAR definitions ========== */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef _Bool scalar_bool_t;
typedef void scalar_void_t;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double_t;
typedef __complex__ float complex_float_t;
typedef int __attribute__((vector_size(16))) vector_int_t;
typedef float __attribute__((vector_size(32))) vector_float_t;

/* Builtin types */
typedef __builtin_va_list va_list_t;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT definitions ========== */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with attributes */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    double field2;
};

/* Recursive struct for complex type graph */
struct recursive_struct {
    int data;
    struct recursive_struct *next;  /* Pointer to self */
    struct forward_declared_struct *fwd_ptr;  /* Pointer to undefined */
};

/* Struct with incomplete array (flexible array member) */
struct with_incomplete_array {
    int count;
    int data[];  /* TYPE_ARRAY with unknown bound */
};

/* Nested struct with union */
struct outer_struct {
    int tag;
    union {
        struct {
            int x;
            float y;
        } point;
        struct {
            char *name;
            int id;
        } info;
    } value;
    struct inner_struct {
        short s;
        long l;
    } inner;
};

/* ========== TYPE_UNION definitions ========== */
union basic_union {
    int as_int;
    float as_float;
    double as_double;
    void *as_ptr;
};

union __attribute__((packed)) packed_union {
    char data[8];
    long long as_ll;
    double as_double;
};

/* ========== TYPE_POINTER definitions ========== */
typedef int *int_ptr_t;
typedef struct basic_struct *struct_ptr_t;
typedef void (*generic_func_ptr_t)(void);
typedef const volatile int *cv_int_ptr_t;

/* Pointer to pointer */
typedef int **int_double_ptr_t;
typedef struct recursive_struct ***recursive_triple_ptr_t;

/* ========== TYPE_ARRAY definitions ========== */
typedef int fixed_array_t[10];
typedef char string_array_t[5][20];
typedef struct basic_struct struct_array_t[3];

/* Incomplete array types */
typedef int incomplete_array_t[];
extern int external_array[];

/* Multi-dimensional */
typedef int matrix_t[3][4][5];

/* ========== TYPE_CALLBACK (function pointer) definitions ========== */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(struct basic_struct *, int);
typedef char *(*string_transform_t)(const char *);
typedef int (*varargs_func_t)(int, ...);

/* Complex callback with pointer to callback */
typedef binary_op_t (*callback_selector_t)(int);

/* ========== Complex interdependent types ========== */
struct type_graph_node {
    int id;
    struct type_graph_node *neighbors[4];  /* Array of pointers */
    union {
        int int_val;
        void *ptr_val;
        binary_op_t func_val;
    } payload;
};

/* Opaque struct definition (was forward declared) */
struct opaque {
    void *data;
    int size;
    struct opaque *next;
};

/* Forward declared struct definition */
struct forward_declared_struct {
    int magic;
    struct recursive_struct *recursive;
    struct opaque *opaque_ptr;
};

/* ========== C++ specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should trigger TYPE_LANG_STRUCT classification */
    struct cpp_lang_struct {
        int x;
        double y;
        
        /* Inline method to make it C++ specific */
        int get_x() const { return x; }
    } __attribute__((transaction_safe));
    
    /* Another with GCC attributes */
    class __attribute__((aligned(64))) cpp_aligned_class {
    public:
        int data[16];
        virtual void method() {}
    };
}
#endif

/* ========== Transaction-safe struct ========== */
struct __attribute__((transaction_safe)) transaction_safe_struct {
    int counter;
    void *data;
};

/* ========== Final complex type combining everything ========== */
typedef struct {
    struct basic_struct base;
    union basic_union value;
    fixed_array_t numbers;
    callback_t handler;
    struct type_graph_node *graph;
    incomplete_array_t flexible_part;
} mega_composite_t;

/* Function declarations using the types */
void process_struct(struct basic_struct *s);
int calculate(binary_op_t op, int a, int b);
struct type_graph_node *create_graph(void);
void traverse_recursive(struct recursive_struct *rs);

#endif /* VARIED_TYPES_H */
