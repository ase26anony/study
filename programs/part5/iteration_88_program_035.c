#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* TYPE_UNDEFINED initially */
typedef struct incomplete incomplete_t; /* TYPE_UNDEFINED initially */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int;            /* TYPE_SCALAR */
typedef char scalar_char;          /* TYPE_SCALAR */
typedef double scalar_double;      /* TYPE_SCALAR */
typedef _Bool scalar_bool;         /* TYPE_SCALAR */

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;  /* TYPE_SCALAR */
typedef __complex__ float complex_float;    /* TYPE_SCALAR */
typedef int __attribute__((vector_size(16))) vector_int; /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
typedef const char* string_type;   /* TYPE_STRING */
typedef char* mutable_string;      /* TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct basic_struct {
    int x;
    double y;
    char z;
};                                 /* TYPE_STRUCT */

/* ========== TYPE_USER_STRUCT ========== */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
};                                 /* TYPE_USER_STRUCT (due to packed) */

struct __attribute__((aligned(64))) aligned_struct {
    int data[16];
    long long extra;
};                                 /* TYPE_USER_STRUCT (due to aligned) */

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    double field2;
    char field3;
};                                 /* TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_char[4];
};                                 /* TYPE_UNION */

union __attribute__((packed)) packed_union {
    long long ll;
    double d;
    char bytes[8];
};                                 /* TYPE_UNION (packed may make it TYPE_USER_STRUCT) */

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;              /* TYPE_POINTER */
typedef struct basic_struct* struct_ptr; /* TYPE_POINTER */
typedef void* void_ptr;            /* TYPE_POINTER */
typedef const volatile int* cv_int_ptr; /* TYPE_POINTER */

/* Recursive pointer type */
struct recursive_struct {
    int data;
    struct recursive_struct* next; /* TYPE_POINTER to same struct */
};

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];       /* TYPE_ARRAY */
typedef char string_array[256];    /* TYPE_ARRAY */
typedef struct basic_struct struct_array[5]; /* TYPE_ARRAY */

/* Incomplete array type */
struct with_incomplete_array {
    int count;
    int data[];                    /* TYPE_ARRAY (incomplete) */
};

/* Multi-dimensional array */
typedef int matrix[3][4];          /* TYPE_ARRAY */

/* ========== TYPE_CALLBACK ========== */
typedef int (*simple_callback)(void); /* TYPE_CALLBACK */
typedef void (*complex_callback)(struct basic_struct*, int, ...); /* TYPE_CALLBACK */
typedef int (*comparator)(const void*, const void*); /* TYPE_CALLBACK */

/* Callback that takes a pointer to struct */
typedef void (*struct_processor)(struct recursive_struct*); /* TYPE_CALLBACK */

/* ========== TYPE_LANG_STRUCT (C++ specific) ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_lang_struct {
        int cpp_field;
        virtual void method() = 0;
    };
}
#endif

/* Alternative: GCC transaction attribute */
struct __attribute__((transaction_safe)) transaction_struct {
    int tx_data;
    void (*tx_callback)(void);     /* TYPE_CALLBACK inside struct */
};                                 /* May become TYPE_LANG_STRUCT */

/* ========== Complex Nested Types ========== */
struct container {
    struct basic_struct embedded;  /* TYPE_STRUCT */
    union basic_union choice;      /* TYPE_UNION */
    int_ptr numbers;               /* TYPE_POINTER */
    fixed_array buffer;            /* TYPE_ARRAY */
    simple_callback handler;       /* TYPE_CALLBACK */
};

/* Union with array of pointers */
union pointer_container {
    struct basic_struct* struct_ptrs[10];  /* TYPE_ARRAY of TYPE_POINTER */
    void* void_ptrs[10];           /* TYPE_ARRAY of TYPE_POINTER */
};

/* ========== Complete previously undefined types ========== */
struct opaque {
    void* secret;
    int hidden;
};

struct forward_declared_struct {
    struct opaque* ptr_to_opaque;  /* TYPE_POINTER */
    incomplete_t* ptr_to_incomplete; /* TYPE_POINTER */
};

struct incomplete {
    int value;
    struct forward_declared_struct* link; /* TYPE_POINTER */
};

/* ========== Builtin types ========== */
typedef __builtin_va_list va_list_type; /* TYPE_SCALAR or special */
typedef __builtin_va_list* va_list_ptr; /* TYPE_POINTER */

/* ========== Function declarations ========== */
void process_types(struct container* c);
int compare_values(const void* a, const void* b);

#endif /* VARIED_TYPES_H */
