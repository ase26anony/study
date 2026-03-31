#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar types */
typedef char scalar_char;
typedef int scalar_int;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* GNU extensions for scalar types */
typedef __complex__ double complex_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;
typedef __builtin_va_list va_list_scalar;

/* TYPE_STRING: String type */
typedef const char* string_type;

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

struct __attribute__((aligned(32))) aligned_struct {
    long data[8];
};

/* TYPE_UNION: Various unions */
union basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct regular_struct* struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char* cv_ptr;

/* TYPE_ARRAY: Various array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_length_array[0];
typedef int multidimensional_array[5][7];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*callback_with_struct)(struct regular_struct*);
typedef int (*variadic_func)(int, ...);

/* Recursive and mutually recursive types for complex graphs */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Self-reference */
    struct forward_declared* fwd; /* Forward reference */
};

/* Now define the forward declared struct */
struct forward_declared {
    struct recursive_node* node;
    struct opaque* opaque_ptr;
};

/* Opaque struct definition (after pointer declarations) */
struct opaque {
    int secret;
    char data[];
};

/* Union containing array of pointers */
union pointer_container {
    struct regular_struct* struct_ptrs[5];
    void* generic_ptrs[10];
    int* int_ptrs[8];
};

/* Struct with incomplete array as last member */
struct flexible_array {
    size_t count;
    int data[];  /* Incomplete array */
};

/* Nested complex type */
struct container {
    struct regular_struct regular;
    union basic_union u;
    fixed_array arr;
    binary_op op;
    struct container* next;
};

/* C++ specific structures for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific {
        int x;
        virtual void method() = 0;
    } __attribute__((transaction_safe));
    
    class gcc_class {
    private:
        int private_data;
    public:
        virtual ~gcc_class() {}
        __attribute__((always_inline)) void inline_method() {}
    };
}
#endif

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int atomic_value;
    void (*atomic_callback)(void);
};

/* Complex callback type */
typedef struct recursive_node* (*node_factory)(int, struct forward_declared*);

#endif /* VARIED_TYPES_H */
