#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;
typedef __builtin_va_list va_list_scalar;

/* TYPE_STRING: String types */
typedef const char* string_ptr;
typedef char* mutable_string;

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

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

/* TYPE_UNION: Various unions */
union data_union {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef struct regular_struct* struct_ptr;
typedef union data_union* union_ptr;
typedef int* int_ptr;
typedef void (*generic_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef struct regular_struct struct_array[5];
typedef int multi_dim_array[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(struct regular_struct*, void*);
typedef char* (*string_formatter)(const char*, ...);

/* Recursive and mutually recursive types */
struct recursive_node {
    int data;
    struct recursive_node* next;  /* Self-referential pointer */
    struct recursive_node* prev;
};

struct graph_node_a;
struct graph_node_b;

struct graph_node_a {
    int value;
    struct graph_node_b* connection;
};

struct graph_node_b {
    float value;
    struct graph_node_a* connection;
    struct graph_node_a* alternate;
};

/* Struct with flexible array member (incomplete array) */
struct flex_array {
    size_t length;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    void* internal_data;
    int secret_code;
};

/* Forward declared struct definition */
struct forward_declared {
    struct opaque* related;
    int counter;
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int balance;
    char account_id[20];
};

/* C++ compatible section for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class CppClass {
    private:
        int private_data;
    public:
        virtual void method() = 0;
        virtual ~CppClass() {}
    };
    
    struct CppCompatibleStruct {
        int x;
        double y;
        CppClass* interface;
    };
}
#endif

/* Complex nested type combining multiple classifications */
struct master_container {
    struct regular_struct regular;      /* TYPE_STRUCT */
    union data_union variant;           /* TYPE_UNION */
    struct_ptr ptr_member;              /* TYPE_POINTER */
    fixed_array numbers;                /* TYPE_ARRAY */
    binary_op operation;                /* TYPE_CALLBACK */
    struct flex_array* flexible;        /* TYPE_POINTER -> TYPE_STRUCT with TYPE_ARRAY */
    struct recursive_node* list_head;   /* Recursive pointer */
};

/* Callback that uses multiple complex types */
typedef void (*complex_callback)(
    struct master_container*,
    struct graph_node_a*,
    binary_op,
    va_list_scalar
);

#endif /* VARIED_TYPES_H */
