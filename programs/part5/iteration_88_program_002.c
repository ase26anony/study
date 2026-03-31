#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration - will be TYPE_UNDEFINED initially */
struct opaque;
struct forward_declared;

/* ========== TYPE_SCALAR ========== */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __complex__ float complex_float;

/* GNU vector extension */
typedef int v4si __attribute__((vector_size(16)));

/* ========== TYPE_STRING ========== */
typedef const char* string_type;
typedef char* mutable_string;

/* ========== TYPE_STRUCT with various attributes ========== */
/* Regular struct */
struct regular_struct {
    int x;
    double y;
    char z;
};

/* Packed struct - likely TYPE_USER_STRUCT */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

/* Struct with designated initializer attribute */
struct __attribute__((designated_init)) designated_struct {
    int field1;
    double field2;
};

/* Struct with aligned attribute */
struct __attribute__((aligned(32))) aligned_struct {
    long data[4];
    char padding;
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
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
        double value;
    } double_data;
};

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;
typedef struct regular_struct* struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char* cv_ptr;

/* Pointer to pointer */
typedef int** int_ptr_ptr;
typedef struct regular_struct*** struct_ptr_ptr_ptr;

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];
typedef char string_array[256];
typedef struct regular_struct struct_array[5];

/* Multi-dimensional array */
typedef int matrix[3][3];
typedef char cube[2][2][2];

/* ========== TYPE_CALLBACK ========== */
typedef int (*binary_op)(int, int);
typedef void (*callback_with_context)(void* context, int value);
typedef int (*va_list_func)(int count, ...);

/* Complex callback signature */
typedef struct regular_struct* (*struct_factory)(int id, const char* name);
typedef void (*destructor_func)(void* obj);

/* ========== Recursive and Nested Types ========== */
/* Self-referential struct */
struct recursive_node {
    int data;
    struct recursive_node* next;
    struct recursive_node* prev;
};

/* Mutually recursive types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_a array_of_a[3];
};

/* Union containing array of pointers */
union container_union {
    struct type_a* a_ptrs[5];
    struct type_b* b_ptrs[3];
    void* generic_ptrs[10];
};

/* Struct with incomplete array (flexible array member) */
struct flex_array {
    int length;
    int data[];  /* TYPE_ARRAY with unknown bound */
};

/* Struct with pointer to callback */
struct with_callback {
    int id;
    binary_op operation;
    callback_with_context notify;
};

/* ========== Opaque struct definition ========== */
struct opaque {
    void* internal_data;
    int secret;
    struct opaque* next;
};

/* ========== Complex nested type ========== */
struct nested_mess {
    union {
        struct {
            int a;
            int b;
        } s;
        long long ll;
    } u;
    
    struct nested_mess* children[4];
    void (*processor)(struct nested_mess*);
    
    struct {
        int tag;
        union {
            int i;
            float f;
            void* p;
        } value;
    } tagged;
};

/* ========== Built-in types ========== */
typedef __builtin_va_list va_list_type;
typedef __builtin_va_list* va_list_ptr;

/* ========== Function declarations ========== */
void use_types_in_other_unit(struct recursive_node* node);
int process_opaque(struct opaque* op);
struct flex_array* create_flex_array(int size);

/* ========== C++ specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should generate TYPE_LANG_STRUCT */
    struct cpp_like_struct {
        int x;
        double y;
        
        #ifdef __GNUC__
        __attribute__((transaction_safe))
        #endif
        void method();
    };
    
    /* Another with transaction_safe attribute */
    struct __attribute__((transaction_safe)) transaction_struct {
        int counter;
        void* data;
    };
}
#endif

/* Attribute that might trigger TYPE_USER_STRUCT */
struct __attribute__((scalar_storage_order("big-endian"))) big_endian_struct {
    int first;
    char second;
    short third;
};

/* Final forward declaration now defined */
struct forward_declared {
    int finally_defined;
    struct opaque* link;
};

#endif /* VARIED_TYPES_H */
