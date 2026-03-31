#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete incomplete_t;

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int;            /* Simple scalar typedef */
typedef unsigned long scalar_ulong;
typedef _Bool scalar_bool;
typedef char scalar_char;

/* GCC-specific scalar extensions */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_scalar;

/* ========== TYPE_STRING ========== */
typedef const char* string_const;
typedef char* string_mutable;

/* ========== TYPE_POINTER ========== */
typedef scalar_int* int_ptr;
typedef void* void_ptr;
typedef struct opaque* opaque_ptr;
typedef int (*func_ptr)(void);

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_array[0];
typedef int (*array_of_func_ptrs[5])(void);

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*callback_simple)(void);
typedef void (*callback_complex)(struct opaque*, int, ...);
typedef int (*callback_with_return)(const char*, ...);

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int a;
    char b;
    double c;
};

/* TYPE_USER_STRUCT with attributes */
struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    long z;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    char field2;
    double field3;
};

/* Struct with incomplete array (flexible array member) */
struct with_flex_array {
    int count;
    int data[];  /* TYPE_ARRAY classification */
};

/* Recursive struct (self-referential pointer) */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* TYPE_POINTER to same struct */
};

/* Mutually recursive structs */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;  /* Forward reference */
};

struct type_b {
    int id;
    struct type_a* partner;
};

/* ========== TYPE_UNION ========== */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    struct packed_struct as_packed;
    callback_simple as_callback;
    int array[4];
};

/* ========== Complete the forward declarations ========== */
struct opaque {
    int magic;
    struct forward_declared_struct* link;
    incomplete_array flex;  /* Incomplete array member */
};

struct forward_declared_struct {
    int value;
    struct opaque* backlink;
};

/* ========== Complex nested type ========== */
struct container {
    struct simple_struct nested_struct;
    union complex_union nested_union;
    fixed_array numbers;
    struct container* self_ptr;
    struct type_a* a_ptr;
    struct type_b* b_ptr;
    callback_complex callback_field;
};

/* ========== C++ specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_lang_struct {
        int cpp_field;
        virtual void method() {}
    } __attribute__((transaction_safe));
    
    class cpp_class {
    public:
        int member;
        virtual ~cpp_class() {}
    };
}
#endif

/* Transaction-safe attribute for potential TYPE_LANG_STRUCT */
struct __attribute__((transaction_safe)) transaction_struct {
    int atomic_value;
    void* atomic_ptr;
};

/* ========== Function declarations ========== */
void use_types_in_other_file(struct recursive_struct* rs);
int process_container(struct container* cont);

#endif /* VARIED_TYPES_H */
