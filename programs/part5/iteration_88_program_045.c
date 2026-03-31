#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr_t;

/* TYPE_SCALAR: Various scalar typedefs */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;

/* TYPE_STRING: String typedef */
typedef const char *string_type;

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((designated_init)) designated_init_struct {
    int x;
    double y;
    char z;
};

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int id;
    float value;
    char name[32];
};

/* TYPE_UNION: Unions */
union data_union {
    int as_int;
    float as_float;
    double as_double;
    void *as_ptr;
};

/* TYPE_ARRAY: Various array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_length_array[0];
typedef int multi_dim_array[5][10];

/* TYPE_POINTER: Pointer types */
typedef int *int_ptr;
typedef struct regular_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef int (*const const_func_ptr)(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void *context, int event_id);
typedef int (*va_func)(int, ...);

/* Recursive and mutually recursive types */
struct recursive_node {
    int data;
    struct recursive_node *next;  /* TYPE_POINTER to self */
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
union pointer_container {
    struct graph_node *node_array[5];
    struct graph_edge *edge_array[10];
    void *generic_ptrs[20];
};

/* Struct with incomplete array at end (flexible array member) */
struct flexible_struct {
    int count;
    double average;
    int data[];  /* TYPE_ARRAY (incomplete) */
};

/* Opaque pointer type (TYPE_UNDEFINED initially) */
extern struct opaque *global_opaque;

/* Now define the previously opaque struct */
struct opaque {
    int magic;
    void *data;
    struct opaque *next;
};

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* C++ specific for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        virtual void method() = 0;
        virtual ~cpp_class() {}
    };
    
    struct __attribute__((transaction_safe)) transaction_struct {
        int value;
        void update(int new_val) __attribute__((transaction_safe));
    };
}
#endif

/* Complex nested type */
typedef struct {
    struct {
        int x;
        int y;
    } point;
    union {
        int int_val;
        float float_val;
    } data;
    binary_op operation;
    fixed_array buffer;
} complex_nested_type;

/* Enumeration (treated as scalar by gengtype) */
typedef enum {
    STATE_IDLE,
    STATE_ACTIVE,
    STATE_ERROR
} state_t;

/* Function declarations using the types */
void process_struct(struct regular_struct *s);
int calculate(binary_op op, int a, int b);
struct recursive_node *create_list(int count);

#endif /* VARIED_TYPES_H */
