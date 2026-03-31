#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;  /* TYPE_UNDEFINED initially */
struct forward_declared_struct;

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int my_int;  /* Simple scalar typedef */
typedef __complex__ double complex_double;  /* GCC complex type */
typedef int __attribute__((vector_size(16))) int_vector;  /* GCC vector type */
typedef __builtin_va_list va_list_type;  /* Builtin va_list */

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*callback_func)(int, void*);
typedef void (*complex_callback)(struct forward_declared_struct*, va_list_type);

/* ========== TYPE_STRUCT with various attributes ========== */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int x;
    double y;
    char z;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct __attribute__((aligned(32))) {
    long long data[4];
    char metadata[16];
} user_struct_aligned;

typedef struct __attribute__((packed, aligned(8))) {
    unsigned char flags;
    unsigned int value;
} user_struct_packed;

/* ========== TYPE_UNION ========== */
union data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* ========== TYPE_POINTER ========== */
typedef my_int* int_ptr;
typedef struct forward_declared_struct** double_ptr_to_struct;
typedef callback_func (*func_ptr_ptr);  /* Pointer to function pointer */

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];
typedef int incomplete_array[];  /* Incomplete array type */
typedef int (*array_of_func_ptrs[5])(void);

/* ========== Recursive and Nested Types ========== */
struct forward_declared_struct {
    int id;
    struct forward_declared_struct* next;  /* Recursive pointer */
    union data_union data;
    callback_func handler;
};

struct complex_nested {
    struct {
        int x;
        struct forward_declared_struct* ptr;
    } inner;
    union {
        fixed_array arr;
        user_struct_aligned user;
    } container;
    array_of_func_ptrs funcs;
};

/* ========== Struct with incomplete array ========== */
struct flexible_array {
    int count;
    int data[];  /* Incomplete/Flexible array member */
};

/* ========== Opaque struct definition (after forward declaration) ========== */
struct opaque {
    void* secret;
    int hidden;
};

/* ========== Complex interconnected types ========== */
typedef struct node {
    struct node* left;
    struct node* right;
    union data_union value;
    void (*visitor)(struct node*);
} tree_node;

/* ========== TYPE_LANG_STRUCT (C++ specific) ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific_struct {
        int cpp_member;
        virtual void method() = 0;
    } __attribute__((transaction_safe));
}
#else
/* Alternative for C: use transaction_safe attribute */
struct __attribute__((transaction_safe)) transaction_struct {
    int safe_data;
    void (*safe_callback)(void);
};
#endif

/* ========== Multi-dimensional arrays ========== */
typedef int matrix[3][3];
typedef int (*callback_matrix[2][2])(int, int);

/* ========== Bitfield struct ========== */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_member;
};

#endif /* VARIED_TYPES_H */
