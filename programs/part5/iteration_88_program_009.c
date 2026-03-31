#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED ========== */
/* Forward declarations that create undefined types initially */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr_t;

/* ========== TYPE_SCALAR ========== */
/* Various scalar type definitions */
typedef char byte_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;

/* GCC extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_t;

/* ========== TYPE_STRING ========== */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT ========== */
/* Basic struct */
struct basic_struct {
    int x;
    float y;
    char z;
};

/* Packed struct for TYPE_USER_STRUCT */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
};

/* Aligned struct for TYPE_USER_STRUCT */
struct __attribute__((aligned(64))) aligned_struct {
    long data[8];
};

/* Struct with designated initializer attribute */
struct __attribute__((designated_init)) designated_struct {
    int id;
    const char *name;
    float value;
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    void *as_ptr;
    char as_bytes[8];
};

/* Tagged union */
union tagged_union {
    struct {
        int type;
    } header;
    struct {
        int type;
        int value;
    } int_data;
    struct {
        int type;
        float value;
    } float_data;
};

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
typedef int *int_ptr_t;
typedef const void *const_void_ptr_t;
typedef volatile char *volatile_char_ptr_t;
typedef struct basic_struct *struct_ptr_t;
typedef union basic_union *union_ptr_t;

/* Pointer to pointer */
typedef int **int_ptr_ptr_t;

/* ========== TYPE_ARRAY ========== */
/* Fixed-size arrays */
typedef int fixed_array_t[10];
typedef struct basic_struct struct_array_t[5];

/* Multi-dimensional array */
typedef int matrix_t[3][3];

/* Incomplete array type (in struct) */
struct with_incomplete_array {
    int count;
    int data[];  /* TYPE_ARRAY */
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(int, const char*, ...);
typedef int (*struct_callback_t)(struct basic_struct*);
typedef void (*recursive_callback_t)(void (*)(void));

/* ========== Recursive and Nested Types ========== */
/* Self-referential struct */
struct recursive_node {
    int value;
    struct recursive_node *next;  /* TYPE_POINTER to same struct */
};

/* Mutually recursive types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b *partner;  /* Forward reference */
};

struct type_b {
    int id;
    struct type_a *partner;  /* Completes the cycle */
};

/* Complex nested type */
struct container {
    struct basic_struct element;
    union basic_union variant;
    int_ptr_t numbers[5];
    simple_callback_t handler;
};

/* Union with array of pointers */
union pointer_union {
    struct basic_struct *struct_ptrs[4];
    int *int_ptrs[8];
    void *generic_ptrs[2];
};

/* ========== Opaque type definition ========== */
struct opaque {
    void *internal_data;
    int (*process)(struct opaque*);  /* Callback with self-reference */
};

/* ========== C++ specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should trigger TYPE_LANG_STRUCT */
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

/* ========== Function declarations ========== */
void use_types_in_other_file(struct recursive_node *node);

#endif /* VARIED_TYPES_H */
