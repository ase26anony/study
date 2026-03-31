#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* Will be TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct opaque *opaque_ptr_t;

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef _Bool scalar_bool_t;
typedef float scalar_float_t;
typedef double scalar_double_t;

/* GNU Extensions for scalar types */
typedef __complex__ double complex_scalar_t;
typedef int __attribute__((vector_size(16))) vector_scalar_t;
typedef __builtin_va_list va_list_scalar_t;

/* ========== TYPE_STRING ========== */
typedef const char *string_ptr_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT / Basic Structs ========== */
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
    double field2;
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    void *as_ptr;
    char as_bytes[8];
};

/* ========== TYPE_POINTER ========== */
typedef int *int_ptr_t;
typedef struct basic_struct *struct_ptr_t;
typedef void (*generic_func_ptr_t)(void);

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array_t[10];
typedef int incomplete_array_t[];  /* Incomplete array type */
typedef int (*array_of_ptrs_t)[5]; /* Pointer to array */

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*simple_callback_t)(int, int);
typedef void (*complex_callback_t)(struct basic_struct *, va_list_scalar_t);
typedef int (*recursive_callback_t)(struct recursive_struct *);

/* ========== Complex Nested/Recursive Types ========== */
struct recursive_struct {
    int data;
    struct recursive_struct *next;  /* Self-reference */
    union basic_union value;
};

struct complex_nested {
    struct basic_struct inner;
    union basic_union variant;
    int_ptr_t ptr_array[5];
    struct recursive_struct *recursive_ptr;
    simple_callback_t callback;
};

/* ========== Struct with Flexible Array Member ========== */
struct flex_array_struct {
    int count;
    double data[];  /* Incomplete array as last member */
};

/* ========== Now define previously opaque types ========== */
struct opaque {
    int secret;
    struct forward_declared_struct *link;
};

struct forward_declared_struct {
    struct opaque *back_link;
    complex_scalar_t complex_val;
};

/* ========== Union containing array of pointers ========== */
union pointer_container {
    struct basic_struct *struct_ptrs[4];
    void *generic_ptrs[8];
    recursive_callback_t callbacks[2];
};

/* ========== C++ Specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific_struct {
        int cpp_member;
        virtual void method() = 0;
    } __attribute__((transaction_safe));
}
#else
/* Alternative GCC attribute for C mode */
struct __attribute__((transaction_safe)) transaction_safe_struct {
    int safe_data;
    void (*safe_op)(int);
};
#endif

/* ========== Function Declarations ========== */
void use_types_in_other_unit(struct recursive_struct *rs);
complex_callback_t get_complex_callback(void);

#endif /* VARIED_TYPES_H */
