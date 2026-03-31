#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED (forward declarations) ========== */
struct opaque;                     /* Will be TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */

/* ========== TYPE_SCALAR (typedefs and basic types) ========== */
typedef int my_int;                /* Simple scalar typedef */
typedef unsigned long my_ulong;
typedef _Bool my_bool;
typedef __complex__ double complex_double;  /* GNU extension */
typedef __complex__ float complex_float;
typedef float __attribute__((vector_size(16))) float_vec4;  /* Vector type */

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_ARRAY (various array types) ========== */
typedef int int_array_10[10];
typedef float matrix_3x3[3][3];
extern int incomplete_array[];     /* Incomplete array type */

/* ========== TYPE_POINTER ========== */
typedef my_int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;
typedef int (*func_ptr)(void);

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(void* context, int event_id);
typedef int (*va_func)(int num, ...);

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int x;
    double y;
    char name[32];
};

/* ========== TYPE_USER_STRUCT (with attributes) ========== */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((designated_init));

struct __attribute__((aligned(64))) overaligned_struct {
    double data[8];
    int tag;
};

/* ========== TYPE_UNION ========== */
union data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

union variant {
    struct simple_struct s;
    union data_union u;
    binary_op callback;
};

/* ========== Complex nested/recursive types ========== */
struct recursive_node {
    int value;
    struct recursive_node* next;      /* Pointer to same type */
    struct recursive_node* prev;
};

struct graph_node {
    int id;
    struct graph_node** neighbors;    /* Pointer to pointer array */
    int neighbor_count;
};

struct container {
    struct simple_struct embedded;
    struct simple_struct* ptr_to_struct;
    union data_union data;
    binary_op operation;
    int_array_10 numbers;
    matrix_3x3 transform;
};

/* ========== Struct with incomplete array (flexible array member) ========== */
struct flex_array {
    size_t length;
    int data[];  /* Incomplete array - TYPE_ARRAY */
};

/* ========== Now define previously opaque types ========== */
struct opaque {
    int magic;
    struct forward_declared_struct* link;
    void* secret;
};

struct forward_declared_struct {
    struct opaque* counterpart;
    char label[64];
    union variant content;
};

/* ========== Complex type with all classifications ========== */
struct mega_type {
    /* SCALAR */
    my_int counter;
    complex_double z;
    float_vec4 v;
    
    /* POINTER */
    opaque_ptr hidden;
    generic_ptr user_data;
    
    /* STRUCT */
    struct simple_struct base;
    
    /* UNION */
    union data_union storage;
    
    /* ARRAY */
    matrix_3x3 matrix;
    
    /* CALLBACK */
    event_callback on_event;
    
    /* STRING */
    string_ptr name;
    
    /* Nested recursive reference */
    struct mega_type* self_ptr;
    
    /* Array of pointers */
    struct recursive_node* nodes[10];
    
    /* Variable arguments */
    __builtin_va_list args;  /* Special scalar/struct type */
};

/* ========== Function declarations using the types ========== */
int process_struct(struct simple_struct* s);
void handle_callback(event_callback cb, void* context);
struct recursive_node* create_node(int value);
void traverse_graph(struct graph_node* start);

/* ========== C++ specific section for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* Transaction-safe struct (GCC extension) */
    struct __attribute__((transaction_safe)) transaction_data {
        int id;
        double amount;
        char description[128];
    };
    
    /* Class with virtual functions */
    class BaseClass {
    public:
        virtual void method() = 0;
        virtual ~BaseClass() {}
    };
    
    class DerivedClass : public BaseClass {
    public:
        virtual void method() override {}
        int extra_data;
    };
}
#endif

/* ========== Union with struct containing array of function pointers ========== */
union complex_union {
    struct {
        int type;
        union {
            int (*int_funcs[5])(int);
            void (*void_funcs[5])(void);
        } func_union;
    } tagged;
    char raw[256];
};

#endif /* VARIED_TYPES_H */
