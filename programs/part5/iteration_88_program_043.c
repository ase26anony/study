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
typedef __complex__ double my_complex;
typedef __builtin_va_list my_va_list;
typedef int __attribute__((vector_size(16))) my_vector;

/* TYPE_STRING: String types */
typedef const char *cstring;
typedef char *mutable_string;

/* TYPE_STRUCT: Regular structs */
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

/* TYPE_UNION: Various unions */
union simple_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef int *int_ptr;
typedef struct simple_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct simple_struct struct_array[5];
extern int incomplete_array[];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void *context, int event_id);
typedef struct simple_struct *(*struct_factory)(int);

/* Recursive and mutually recursive types */
struct recursive_node {
    int value;
    struct recursive_node *next;  /* TYPE_POINTER to same struct */
    struct recursive_node *prev;
};

struct graph_node;
struct graph_edge {
    struct graph_node *from;
    struct graph_node *to;
    int weight;
};

struct graph_node {
    int id;
    struct graph_edge **edges;  /* Pointer to array of pointers */
    int edge_count;
};

/* Union with array of pointers */
union container {
    struct recursive_node *node_list[5];
    struct graph_node *graph_nodes[3];
    void *generic_ptrs[10];
};

/* Struct with incomplete array (flexible array member) */
struct flexible_array {
    int count;
    int data[];  /* TYPE_ARRAY incomplete */
};

/* Opaque pointer type */
struct opaque {
    void *internal_data;
    int type_tag;
};

/* Complex nested type */
struct nested_container {
    union container u;
    struct {
        binary_op ops[3];
        event_handler handlers[2];
    } callbacks;
    struct flexible_array *flex;
};

/* Forward declared struct (initially TYPE_UNDEFINED) */
struct forward_declared {
    int magic;
    struct forward_declared *self_ptr;
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* Variadic callback */
typedef int (*variadic_func)(int, ...);

/* Function declarations using the types */
void process_struct(struct simple_struct *s);
struct recursive_node *create_node(int value);
int perform_operation(binary_op op, int a, int b);

/* C++ specific section for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class CppClass {
    private:
        int private_data;
    public:
        virtual void method() = 0;
        static int static_method(int x);
    };
    
    struct CppStruct {
        CppClass *ptr;
        int value;
    };
}
#endif

#endif /* VARIED_TYPES_H */
