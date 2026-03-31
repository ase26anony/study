#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete* incomplete_ptr_t; /* Pointer to undefined */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int_t;          /* Simple scalar typedef */
typedef __complex__ double complex_scalar_t;  /* GNU extension scalar */
typedef __builtin_va_list va_list_scalar_t;   /* Builtin scalar type */
typedef int __attribute__((vector_size(16))) vector_scalar_t; /* Vector type */

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr_t;  /* String pointer type */
typedef char* mutable_string_t;

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array_t[10];     /* Fixed-size array */
typedef int incomplete_array_t[];  /* Incomplete array type */
extern int extern_array[];         /* External incomplete array */

/* ========== TYPE_POINTER ========== */
typedef scalar_int_t* int_ptr_t;   /* Pointer to scalar */
typedef void* generic_ptr_t;       /* Generic pointer */
typedef const volatile int* cv_ptr_t; /* Qualified pointer */

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*simple_callback_t)(int, int);  /* Simple function pointer */
typedef void (*complex_callback_t)(struct opaque*, va_list_scalar_t, ...);
typedef int (*recursive_callback_t)(struct forward_declared_struct*, 
                                    recursive_callback_t); /* Self-referential */

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with attributes */
struct __attribute__((packed, aligned(8))) user_struct {
    int id;
    char name[20];
    long timestamp;
} __attribute__((designated_init));

/* Struct with incomplete array at end */
struct flexible_array_struct {
    int count;
    double data[];  /* Incomplete array */
};

/* Recursive struct with pointer to self */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* Pointer to same type */
    struct forward_declared_struct* forward_ref; /* Pointer to undefined */
};

/* Now define the previously forward-declared struct */
struct forward_declared_struct {
    struct recursive_struct* rec;
    incomplete_array_t flex;  /* Using incomplete array typedef */
};

/* Define the opaque struct */
struct opaque {
    void* secret;
    struct opaque* next_opaque;
};

/* ========== TYPE_UNION ========== */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    struct recursive_struct* as_rec_ptr;
    complex_callback_t as_callback;
    fixed_array_t as_array;
};

/* ========== TYPE_LANG_STRUCT (C++ specific) ========== */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        virtual ~cpp_class() {}
        virtual void method() = 0;
    };
    
    struct __attribute__((transaction_safe)) transaction_struct {
        int transaction_id;
        double amount;
    };
}
#else
/* For C, use transaction_safe attribute if supported */
#if __has_attribute(transaction_safe)
struct __attribute__((transaction_safe)) transaction_struct {
    int transaction_id;
    double amount;
};
#endif
#endif

/* ========== Complex Nested Type ========== */
struct master_container {
    struct simple_struct basic;
    union complex_union variant;
    fixed_array_t numbers;
    incomplete_array_t dynamic;  /* Incomplete array member */
    struct recursive_struct* recursion;
    simple_callback_t callback;
    struct master_container* sibling;  /* Pointer to same type */
};

/* ========== Array of Complex Types ========== */
typedef struct master_container container_array_t[5];
typedef union simple_union union_array_t[][3];  /* 2D incomplete array */

/* ========== Function Declarations ========== */
void use_types_in_other_unit(struct recursive_struct* rs, 
                            complex_callback_t cb);

#endif /* VARIED_TYPES_H */
