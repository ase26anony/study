#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar typedefs */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __complex__ float complex_float;
typedef float __attribute__((vector_size(16))) float_vec;
typedef int __attribute__((vector_size(32))) int_vec;

/* GNU extensions for scalar types */
typedef __builtin_va_list va_list_scalar;
typedef __int128 int128_scalar;
typedef unsigned __int128 uint128_scalar;

/* TYPE_STRING: String typedef */
typedef const char* string_type;

/* TYPE_ARRAY: Various array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_length_array[0];
typedef int variable_len_array[__builtin_choose_expr(1, 5, 0)];

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* void_ptr;
typedef struct opaque* opaque_ptr;
typedef int (*func_ptr)(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_callback)(int, int);
typedef void (*void_callback)(void);
typedef struct recursive_struct* (*struct_callback)(struct recursive_struct*);

/* TYPE_STRUCT: Plain structs */
struct simple_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    double field2;
};

struct __attribute__((aligned(64))) aligned_struct {
    long long data[8];
};

/* TYPE_UNION: Various unions */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
};

/* Complex nested types */
struct recursive_struct {
    int data;
    struct recursive_struct* next;  /* Self-referential pointer */
    union simple_union value;
};

struct container_struct {
    struct recursive_struct rec;
    union simple_union uni;
    fixed_array arr;
    incomplete_array* flex_ptr;
};

/* Struct with flexible array member (incomplete array) */
struct flex_array_struct {
    int count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    void* hidden_data;
    int secret;
};

/* Forward declared struct definition */
struct forward_declared {
    struct recursive_struct* ptr;
    binary_callback cb;
};

/* Nested pointer/array combinations */
typedef struct recursive_struct* recursive_ptr_array[5];
typedef binary_callback callback_array[3];

/* Complex type graph */
struct type_graph_node {
    int id;
    struct type_graph_node* neighbors[4];
    void (*processor)(struct type_graph_node*);
};

/* Union containing array of pointers */
union pointer_container {
    struct recursive_struct* struct_ptrs[10];
    void* void_ptrs[10];
    binary_callback callbacks[5];
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void (*update)(int);
};

/* C++ specific structures for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    public:
        virtual ~cpp_class() {}
        virtual void method() = 0;
    private:
        int private_data;
    };
    
    struct cpp_wrapper {
        cpp_class* obj;
        void (*cleanup)(cpp_class*);
    };
}
#endif

/* Callback that uses multiple types */
typedef void (*complex_callback)(
    struct recursive_struct*,
    union pointer_container*,
    binary_callback
);

/* Function pointer with varargs */
typedef int (*varargs_callback)(int, ...);

/* Nested anonymous struct/union */
struct anonymous_members {
    struct {
        int x;
        int y;
    } point;
    union {
        int as_int;
        float as_float;
    } value;
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int : 0;  /* Force alignment */
    unsigned int flag4 : 8;
};

#endif /* VARIED_TYPES_H */
