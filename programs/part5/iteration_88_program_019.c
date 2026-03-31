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
typedef _Bool my_bool;
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* GNU extensions for scalar types */
typedef __builtin_va_list va_list_type;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* TYPE_STRING: String types */
typedef const char *cstring;
typedef char *mutable_string;

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

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Union definitions */
union simple_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef struct simple_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile int *cv_int_ptr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct simple_struct struct_array[5];
extern int incomplete_array[];
typedef struct {
    int length;
    int data[];  /* Flexible array member */
} flex_array_struct;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void *context, int event_id);
typedef int (*variadic_func)(int count, ...);

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
    struct graph_node_b *connections[4];
};

struct graph_node_b {
    int id;
    struct graph_node_a *primary;
    struct graph_node_a **secondaries;
};

/* Opaque pointer type (TYPE_UNDEFINED initially) */
extern struct opaque *global_opaque;

/* Complete definition of previously opaque struct */
struct opaque {
    int magic;
    void *data;
    struct opaque *next;
};

/* Union with array of pointers */
union complex_union {
    struct graph_node_a *node_array[8];
    binary_op callbacks[4];
    void *generic_ptrs[16];
};

/* Nested struct with function pointer */
struct container {
    int id;
    union complex_union data;
    event_handler on_event;
    struct container *(*clone)(struct container *);
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_safe_struct {
    int counter;
    void *resource;
};

/* C++ specific structures for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class SimpleClass {
    private:
        int value;
    public:
        SimpleClass(int v) : value(v) {}
        virtual ~SimpleClass() {}
        virtual int getValue() const { return value; }
    };
    
    class DerivedClass : public SimpleClass {
    private:
        double extra;
    public:
        DerivedClass(int v, double e) : SimpleClass(v), extra(e) {}
        virtual int getValue() const override { return SimpleClass::getValue() + (int)extra; }
    };
}
#endif

/* Callback that uses the recursive struct */
typedef void (*node_visitor)(struct recursive_node *node, void *user_data);

/* Function declarations using the types */
void process_container(struct container *cont);
int sum_array(const int_array arr);
void register_callback(event_handler handler);

#endif /* VARIED_TYPES_H */
