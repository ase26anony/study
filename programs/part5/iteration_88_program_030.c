#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED (forward declarations) ========== */
struct opaque;                     /* Will be TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */

/* ========== TYPE_SCALAR typedefs ========== */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_STRUCT definitions ========== */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with attributes */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    char field2;
    double field3;
};

/* Struct with incomplete array (TYPE_ARRAY) */
struct flex_array_struct {
    int count;
    int data[];  /* Incomplete array */
};

/* Recursive struct (TYPE_POINTER to own type) */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* Pointer to self */
    struct forward_declared_struct* fwd_ptr;  /* Pointer to undefined */
};

/* Now define the forward-declared struct */
struct forward_declared_struct {
    int id;
    struct recursive_struct* rec_ptr;
};

/* Struct containing various type members */
struct container_struct {
    scalar_int s_int;
    string_ptr str;
    struct basic_struct basic;
    struct packed_struct* packed_ptr;
    int fixed_array[10];  /* Fixed-size array */
    int* dyn_array;       /* Pointer to array */
};

/* ========== TYPE_UNION definitions ========== */
union basic_union {
    int as_int;
    float as_float;
    char* as_string;
    void* as_ptr;
};

union tagged_union {
    struct {
        int type;
        union basic_union data;
    } tagged;
    long long raw;
} __attribute__((packed));

/* ========== TYPE_POINTER typedefs ========== */
typedef int* int_ptr;
typedef struct basic_struct* struct_ptr;
typedef union basic_union* union_ptr;
typedef void (*generic_callback)(void);

/* Pointer to pointer */
typedef int** int_double_ptr;
typedef struct recursive_struct*** recursive_triple_ptr;

/* ========== TYPE_ARRAY typedefs ========== */
typedef int int_array_10[10];
typedef struct basic_struct struct_array_5[5];
typedef char* string_array[8];

/* Multi-dimensional arrays */
typedef int matrix_3x3[3][3];
typedef float tensor_2x2x2[2][2][2];

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*binary_op)(int, int);
typedef void (*struct_handler)(struct container_struct*);
typedef struct recursive_struct* (*allocator_fn)(size_t);
typedef int (*varargs_fn)(int, ...);

/* Complex callback signature */
typedef void (*complex_callback)(
    struct container_struct*,
    union tagged_union*,
    binary_op,
    va_list_type
);

/* ========== Opaque struct definition (after forward declaration) ========== */
struct opaque {
    void* data;
    size_t size;
    struct opaque* next;
};

/* ========== Extern C++ block for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_like_struct {
        int value;
        void method() {}
    } __attribute__((transaction_safe));
    
    class SimpleClass {
    private:
        int private_data;
    public:
        SimpleClass() : private_data(0) {}
        int get() const { return private_data; }
        void set(int v) { private_data = v; }
    };
}
#endif

/* Transaction-safe struct (might become TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* ========== Global type references ========== */
extern struct opaque* global_opaque;
extern complex_callback global_callback;
extern matrix_3x3 global_matrix;

#endif /* VARIED_TYPES_H */
