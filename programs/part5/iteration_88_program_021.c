#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar types and typedefs */
typedef int my_int;
typedef char my_char;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef void my_void;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* TYPE_STRING: String types */
typedef const char* cstring;
typedef char* mutable_string;

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef const int* const_int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;
typedef int (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_length_array[0];
typedef int multidimensional_array[5][10];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*callback)(void* data, int value);
typedef int (*variadic_func)(int, ...);
typedef void (*complex_callback)(struct forward_declared*, va_list_type);

/* TYPE_STRUCT: Regular structs */
struct point {
    int x;
    int y;
    int z;
};

struct data_record {
    int id;
    char name[50];
    double values[20];
    struct point location;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_struct {
    int field1;
    char field2;
    double field3;
};

struct __attribute__((aligned(64))) aligned_struct {
    long long data[8];
};

/* TYPE_UNION: Union types */
union variant {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
    char as_string[20];
};

union tagged_union {
    struct {
        int type;
        char data[100];
    } tagged;
    struct {
        long id;
        double value;
    } raw;
};

/* Recursive and mutually recursive types */
struct recursive_node {
    int value;
    struct recursive_node* next;
    struct recursive_node* prev;
};

struct tree_node {
    int data;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

/* Mutually recursive types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;
    struct type_a* next;
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_b* next;
};

/* Struct with flexible array member (incomplete array) */
struct flex_array {
    size_t length;
    int data[];  /* TYPE_ARRAY with incomplete type */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int hidden_data;
    char secret[100];
    struct opaque* next;
};

/* Forward declared struct definition */
struct forward_declared {
    int magic;
    struct forward_declared* self_ref;
    callback handler;
};

/* Complex nested type */
struct container {
    union variant item;
    struct point position;
    fixed_array numbers;
    binary_op operation;
    struct flex_array* flex;
    struct container* next;
};

/* Array of function pointers */
typedef int (*op_array[10])(int, int);

/* C++ mode structs for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    struct cpp_struct {
        int data;
        void method() {}
    };
    
    class cpp_class {
    private:
        int private_data;
    public:
        virtual void virtual_method() = 0;
        void concrete_method() {}
    };
}
#endif

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void* data;
};

/* Complex callback with nested parameters */
typedef struct container* (*factory_func)(
    int count, 
    struct point* points, 
    callback notify
);

#endif /* VARIED_TYPES_H */
