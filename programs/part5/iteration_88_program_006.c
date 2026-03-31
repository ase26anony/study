#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar typedefs */
typedef char byte_t;
typedef int integer_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef void void_t;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) v4si;
typedef float __attribute__((vector_size(32))) v8sf;

/* TYPE_STRING: String typedef */
typedef const char* string_t;

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct opaque* opaque_ptr_t;
typedef void (*generic_func_ptr_t)(void);

/* TYPE_ARRAY: Array typedefs */
typedef int int_array_10[10];
typedef float matrix_3x3[3][3];
typedef char string_array[][32];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* user_data, int result);
typedef int (*va_func_t)(int count, ...);

/* TYPE_UNION: Union definitions */
union basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union tagged_union {
    struct {
        int type;
        union {
            int int_value;
            float float_value;
            char* string_value;
        } data;
    } tagged;
    unsigned char raw[16];
};

/* TYPE_STRUCT: Regular struct definitions */
struct point {
    int x;
    int y;
    int z;
};

struct node {
    int data;
    struct node* next;  /* Recursive pointer */
    struct node* prev;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

struct __attribute__((aligned(32))) aligned_struct {
    double data[4];
    int flags;
};

struct __attribute__((designated_init)) designated_init_struct {
    int mandatory;
    int optional;
    char* name;
};

/* Struct with incomplete array (flexible array member) */
struct flex_array {
    size_t count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* Complex nested struct */
struct container {
    union basic_union storage;
    struct point location;
    int_array_10 buffer;
    callback_t on_complete;
    struct node* item_list;
};

/* Now define previously forward-declared structs */
struct opaque {
    int secret;
    void* handle;
};

struct forward_declared {
    struct opaque* ptr;
    int value;
};

/* Transaction-safe struct (potential TYPE_LANG_STRUCT) */
struct __attribute__((transaction_safe)) transaction_struct {
    int balance;
    int pending;
};

/* Variadic callback */
typedef int (*variadic_callback_t)(const char* fmt, ...);

/* Nested type definitions */
typedef struct {
    int id;
    char name[64];
    union {
        int int_val;
        double dbl_val;
    } value;
} anonymous_typed_t;

/* Function declarations using these types */
void process_callback(callback_t cb, void* data);
struct container* create_container(int size);
void cleanup_opaque(struct opaque* op);

/* Incomplete struct array parameter */
void process_flex_array(struct flex_array* fa);

#endif /* VARIED_TYPES_H */
