#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int my_int;                /* Simple scalar typedef */
typedef char my_char;
typedef _Bool my_bool;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) int_vector;
typedef float __attribute__((vector_size(32))) float_vector;

/* Builtin types */
typedef __builtin_va_list my_va_list;

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_STRUCT / Basic Structs ========== */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* Packed struct for TYPE_USER_STRUCT */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

/* Struct with designated init attribute */
struct __attribute__((designated_init)) designated_struct {
    int field1;
    double field2;
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Tagged union with struct inside */
union tagged_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
        } data;
    } tagged;
    char raw_data[16];
};

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;
typedef struct basic_struct* struct_ptr;
typedef void (*generic_func_ptr)(void);

/* Pointer to pointer */
typedef int** int_ptr_ptr;

/* ========== TYPE_ARRAY ========== */
typedef int int_array[10];
typedef struct basic_struct struct_array[5];

/* Incomplete array type */
struct with_incomplete_array {
    int count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Multi-dimensional array */
typedef int matrix[3][4];

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void* context, int event_id);
typedef struct basic_struct* (*struct_factory)(int);

/* Complex callback with varargs */
typedef int (*printf_like)(const char* format, ...);

/* ========== Recursive and Interconnected Types ========== */

/* Forward declaration for mutual recursion */
struct list_node;

/* Self-referential struct */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* Pointer to own type */
};

/* Mutually recursive types */
struct list_node {
    int data;
    struct tree_node* child;
};

struct tree_node {
    int value;
    struct list_node* siblings;
};

/* Union containing array of pointers */
union pointer_container {
    struct basic_struct* struct_ptrs[5];
    void* generic_ptrs[10];
};

/* ========== Complete the undefined types ========== */
struct opaque {
    void* secret;
    int magic_number;
};

struct forward_declared_struct {
    int defined_now;
    struct opaque* uses_opaque;
};

/* ========== Complex Nested Type ========== */
struct nested_mess {
    union {
        struct {
            int a;
            double b;
        } s;
        long long ll;
    } u;
    
    struct nested_mess* self_ptr;
    binary_op operation;
    
    struct {
        int hidden;
        char data[8];
    } anonymous;
    
    int flexible_array[0];  /* Zero-length array */
};

/* ========== Transaction Safe Struct (potential TYPE_LANG_STRUCT) ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_like_struct {
        int x;
        double y;
        virtual void method() {}  /* Makes it C++-like */
    };
}
#else
/* For C, use transaction_safe attribute */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void* data;
};
#endif

/* ========== Function using all types ========== */
#ifdef __cplusplus
extern "C" {
#endif

void use_varied_types(void);
struct recursive_struct* create_recursive_chain(int length);
int process_nested_mess(struct nested_mess* mess);

#ifdef __cplusplus
}
#endif

#endif /* VARIED_TYPES_H */
