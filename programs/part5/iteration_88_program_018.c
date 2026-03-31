#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int my_int;                /* Simple scalar typedef */
typedef char my_char;
typedef _Bool my_bool;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) int_vector;
typedef float __attribute__((vector_size(32))) float_vector;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
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

/* Struct with incomplete array (TYPE_ARRAY) */
struct flexible_array_struct {
    int count;
    int data[];  /* Incomplete array */
};

/* Recursive struct (TYPE_POINTER within struct) */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* Pointer to own type */
};

/* Struct containing union */
struct struct_with_union {
    int type;
    union {
        int int_val;
        double double_val;
        void* ptr_val;
    } data;
};

/* ========== TYPE_UNION ========== */
union simple_union {
    int as_int;
    double as_double;
    void* as_ptr;
    char as_char[8];
};

/* Union with struct member */
union union_with_struct {
    struct {
        int x, y;
    } point;
    long long bits;
};

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*generic_callback)(void);

/* Pointer to pointer */
typedef int** int_ptr_ptr;
typedef struct recursive_struct*** recursive_ptr_ptr_ptr;

/* ========== TYPE_ARRAY ========== */
typedef int int_array_10[10];
typedef struct simple_struct struct_array_5[5];
typedef int multi_dim_array[3][4][5];

/* Incomplete array type */
typedef int incomplete_array[];

/* ========== TYPE_CALLBACK ========== */
typedef void (*simple_callback)(int, char);
typedef int (*complex_callback)(struct simple_struct*, union simple_union*, va_list_type);
typedef struct recursive_struct* (*recursive_callback)(struct recursive_struct*, int);

/* Callback returning function pointer */
typedef void (*(*meta_callback)(int))(void);

/* ========== Complex Nested Types ========== */
struct complex_nested {
    struct simple_struct base;
    union simple_union variant;
    int_array_10 numbers;
    struct complex_nested* self_ref;
    simple_callback cb;
};

/* Struct containing array of pointers to another struct */
struct pointer_array_container {
    int count;
    struct simple_struct* items[20];
};

/* ========== Opaque struct definition (after forward declaration) ========== */
struct opaque {
    void* data;
    int size;
    struct opaque* next;
};

/* ========== Language-specific structures ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific_struct {
        int cpp_field;
        virtual void method() {}
    } __attribute__((transaction_safe));
}
#else
/* For C, use transaction attribute directly */
struct __attribute__((transaction_safe)) transaction_safe_struct {
    int safe_field;
    void* safe_ptr;
};
#endif

/* ========== TYPE_LANG_STRUCT candidates ========== */
struct __attribute__((may_alias)) aliasing_struct {
    int x;
    double y;
};

/* Struct with cleanup attribute */
struct __attribute__((cleanup(free_cleanup))) cleanup_struct {
    void* data;
};

/* Helper for cleanup attribute */
static inline void free_cleanup(void** ptr) {
    if (ptr && *ptr) {
        /* In real code, would free memory */
    }
}

/* ========== Function declarations using the types ========== */
void process_struct(struct simple_struct* s);
int calculate_sum(int_array_10 arr);
struct recursive_struct* create_recursive_chain(int depth);
void register_callback(simple_callback cb);

/* ========== Global variables for multi-file testing ========== */
extern struct simple_struct global_struct;
extern union simple_union global_union;
extern simple_callback global_callback;

#endif /* VARIED_TYPES_H */
