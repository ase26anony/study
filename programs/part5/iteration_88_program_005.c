#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr;

/* TYPE_SCALAR: Various scalar types and typedefs */
typedef int my_int;
typedef char my_char;
typedef long long my_llong;
typedef _Bool my_bool;
typedef float my_float;
typedef double my_double;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* TYPE_STRING: String types */
typedef const char *cstring;
typedef char *mutable_string;

/* TYPE_STRUCT: Regular structs */
struct simple_struct {
    int a;
    char b;
    double c;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    long z;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_struct {
    int field1;
    char field2;
    double field3;
};

/* TYPE_UNION: Regular unions */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef struct simple_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef char char_array[];
extern int incomplete_array[];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(struct simple_struct*, void*);
typedef int (*variadic_func)(int, ...);

/* Recursive and mutually recursive types for complex graphs */
struct recursive_node {
    int value;
    struct recursive_node *next;  /* Pointer to self */
    struct recursive_node *prev;
};

struct graph_node_a;
struct graph_node_b;

struct graph_node_a {
    int id;
    struct graph_node_b *link;
};

struct graph_node_b {
    int id;
    struct graph_node_a *link;
    struct graph_node_a *links[5];  /* Array of pointers */
};

/* Struct with incomplete array as last member */
struct flexible_array {
    int count;
    double average;
    int data[];  /* Incomplete array */
};

/* Opaque pointer type (TYPE_UNDEFINED initially) */
extern struct opaque *global_opaque;

/* Now define the opaque struct */
struct opaque {
    void *data;
    int size;
    struct opaque *next;
};

/* Union containing array of pointers */
union pointer_container {
    void *single_ptr;
    void *ptr_array[8];
    struct simple_struct *struct_ptrs[4];
};

/* Nested complex type */
struct container {
    struct simple_struct inner;
    union simple_union choice;
    int_array numbers;
    binary_op operation;
    struct container *next;
};

/* For TYPE_LANG_STRUCT - use C++ if available */
#ifdef __cplusplus
extern "C++" {
    class CppClass {
    private:
        int private_data;
    public:
        virtual ~CppClass() {}
        virtual void method() = 0;
    };
    
    struct __attribute__((transaction_safe)) transaction_struct {
        int value;
        void update(int new_val) __attribute__((transaction_safe));
    };
}
#else
/* Use GCC transaction attribute for C */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
};
#endif

/* Function declarations using the types */
void process_struct(struct simple_struct *s);
int sum_array(const int_array arr);
void register_callback(callback_t cb);

#endif /* VARIED_TYPES_H */
