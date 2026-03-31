#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int;            /* TYPE_SCALAR */
typedef float scalar_float;        /* TYPE_SCALAR */
typedef _Bool scalar_bool;         /* TYPE_SCALAR */
typedef char scalar_char;          /* TYPE_SCALAR */

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;      /* TYPE_SCALAR */
typedef __complex__ float complex_float;        /* TYPE_SCALAR */
typedef int __attribute__((vector_size(16))) vector_int;  /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
typedef const char* string_type;   /* TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int x;
    float y;
    char z;
};

/* ========== TYPE_USER_STRUCT (with attributes) ========== */
struct __attribute__((packed, aligned(8))) packed_struct {
    int a;
    char b;
    double c;
} __attribute__((designated_init));

struct __attribute__((aligned(32))) overaligned_struct {
    long long data[4];
};

/* ========== TYPE_UNION ========== */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;              /* TYPE_POINTER */
typedef struct simple_struct* struct_ptr;  /* TYPE_POINTER */
typedef void (*generic_func_ptr)(void);    /* TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];       /* TYPE_ARRAY */
extern int incomplete_array[];     /* TYPE_ARRAY (incomplete) */

/* Struct with flexible array member */
struct flex_array_struct {
    int count;
    int data[];                    /* Incomplete array inside struct */
};

/* ========== TYPE_CALLBACK ========== */
typedef int (*callback_func)(int, const char*);  /* TYPE_CALLBACK */
typedef void (*va_callback)(int, ...);           /* TYPE_CALLBACK with varargs */

/* Complex callback taking struct pointer */
typedef void (*struct_callback)(struct simple_struct*, int);

/* ========== Recursive and Nested Types ========== */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* Pointer to self */
    struct forward_declared_struct* fwd_ptr;  /* Pointer to undefined */
};

/* Now define the forward-declared struct */
struct forward_declared_struct {
    struct recursive_struct* rec_ptr;
    callback_func cb;
};

/* Union containing array of pointers */
union pointer_container {
    struct simple_struct* struct_ptrs[5];
    int* int_ptrs[10];
    callback_func callbacks[3];
};

/* ========== Complex Nested Struct ========== */
struct nested_complex {
    struct {
        int depth;
        struct nested_complex* parent;
    } inner;
    
    union {
        int tag;
        float value;
    } discriminator;
    
    fixed_array numbers;
    struct_callback handler;
};

/* ========== Built-in Types ========== */
typedef __builtin_va_list va_list_type;  /* Special builtin type */

/* ========== Opaque Pointer Types ========== */
typedef struct opaque* opaque_ptr;  /* Pointer to undefined struct */

/* Now define the opaque struct */
struct opaque {
    void* data;
    int size;
    opaque_ptr next;  /* Recursive opaque pointer */
};

/* ========== C++ Specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should trigger TYPE_LANG_STRUCT classification */
    class cpp_class {
    private:
        int private_data;
    public:
        cpp_class() : private_data(0) {}
        virtual ~cpp_class() {}
        virtual void method() = 0;
    };
    
    /* Class with transaction_safe attribute */
    class __attribute__((transaction_safe)) transaction_class {
        int value;
    public:
        transaction_class(int v) : value(v) {}
        int get_value() const { return value; }
    };
}
#endif

/* ========== Function Types ========== */
/* Function returning struct by value */
struct simple_struct create_struct(int x, float y);

/* Function taking callback */
void register_callback(callback_func cb);

/* Function with complex signature */
int process_data(struct nested_complex* data, 
                 struct_callback cb,
                 va_list_type args);

#endif /* VARIED_TYPES_H */
