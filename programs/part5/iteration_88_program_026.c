#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr_t;

/* TYPE_SCALAR: Various scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __builtin_va_list va_list_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;

/* TYPE_STRING: String types */
typedef const char *string_type;
typedef char *mutable_string;

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed, aligned(4))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((designated_init));

struct __attribute__((aligned(64))) aligned_struct {
    long long data[8];
};

/* TYPE_UNION: Various unions */
union basic_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

union __attribute__((packed)) packed_union {
    char bytes[4];
    int value;
};

/* TYPE_POINTER: Pointer types */
typedef int *int_ptr;
typedef struct regular_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile void *cv_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int (*array_of_ptrs)[5];
typedef int multidim_array[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void *context, int event_id);
typedef int (*va_func)(int, ...);

/* Recursive and interdependent types */
struct recursive_node {
    int data;
    struct recursive_node *next;  /* Self-referential pointer */
    struct recursive_node *prev;
};

struct graph_node {
    int id;
    struct graph_edge **edges;  /* Pointer to array of pointers */
    int edge_count;
};

struct graph_edge {
    struct graph_node *from;
    struct graph_node *to;
    int weight;
};

/* Union with array of pointers */
union container_union {
    struct graph_node *nodes[10];
    struct graph_edge *edges[10];
    void *data[20];
};

/* Incomplete struct with flexible array member */
struct flexible_struct {
    size_t length;
    int data[];  /* Incomplete array */
};

/* Now define the previously opaque struct */
struct opaque {
    int magic;
    struct regular_struct embedded;
    struct forward_declared *link;
};

/* Define the forward declared struct */
struct forward_declared {
    int counter;
    struct opaque *backlink;
};

/* Complex nested type */
typedef struct {
    union container_union storage;
    binary_op operations[5];
    struct flexible_struct *flex;
} nested_type;

/* Callback that uses our types */
typedef void (*complex_callback)(struct recursive_node*, nested_type*, int);

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void (*update)(int);
};

/* For C++ mode: class definition */
#ifdef __cplusplus
extern "C++" {
    class GengtypeTestClass {
    private:
        int private_data;
    protected:
        float protected_data;
    public:
        GengtypeTestClass() : private_data(0), protected_data(0.0f) {}
        virtual ~GengtypeTestClass() {}
        virtual void method() = 0;
        static int static_method(int x) { return x * 2; }
    };
    
    class DerivedClass : public GengtypeTestClass {
    public:
        void method() override {}
        int extra_field;
    };
}
#endif

/* Global callback function type */
typedef void (*global_handler_t)(struct opaque*, va_list_scalar);

#endif /* VARIED_TYPES_H */
