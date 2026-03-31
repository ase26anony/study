#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>
#include <stddef.h>

/* ========== Undefined/Incomplete Types ========== */
struct undefined_struct;      /* TYPE_UNDEFINED */
union undefined_union;        /* TYPE_UNDEFINED */

/* ========== Scalar Types ========== */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== String Types ========== */
typedef char* string_t;
typedef const char* const_string_t;

/* ========== Struct Types ========== */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct complex_struct {
    int id;
    char name[50];
    struct simple_struct nested;
    struct complex_struct* next;  /* Linked list */
    struct complex_struct* prev;
};

/* Anonymous struct within struct */
struct with_anonymous {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
};

/* Packed struct with bitfields */
struct packed_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int count : 24;
    char padding;
} __attribute__((packed));

/* Aligned struct */
struct aligned_struct {
    double data[4];
    long long timestamp;
} __attribute__((aligned(64)));

/* Forward declared struct that will be defined later */
struct forward_declared;

/* ========== User Struct Types ========== */
/* These are typedef'd structs */
typedef struct {
    int x, y;
} point_t;

typedef struct {
    point_t top_left;
    point_t bottom_right;
    char label[32];
} rectangle_t;

/* ========== Union Types ========== */
union simple_union {
    int i;
    float f;
    double d;
    char* s;
};

union tagged_union {
    int type;
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char char_val;
        } data;
    } tagged;
};

/* Anonymous union */
struct with_union {
    int discriminant;
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    };
};

/* ========== Pointer Types ========== */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct complex_struct* complex_ptr_t;
typedef complex_ptr_t* complex_ptr_ptr_t;

/* ========== Array Types ========== */
typedef int int_array_10_t[10];
typedef int int_matrix_3x3_t[3][3];
typedef int int_cube_2x2x2_t[2][2][2];
typedef char* string_array_t[20];
typedef struct simple_struct struct_array_t[5];
typedef int (*func_ptr_array_t[5])(int, int);

/* ========== Callback/Function Pointer Types ========== */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef char* (*string_transform_t)(const char*);
typedef int (*variadic_func_t)(int, ...);
typedef void (*complex_callback_t)(struct complex_struct*, union simple_union);

/* ========== Language Struct Types ========== */
/* These might be used by language-specific extensions */
typedef __builtin_va_list va_list_t;
typedef struct {
    va_list_t args;
    int count;
} va_wrapper_t;

/* Vector types (GNU extension) */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* ========== Complex Type Relationships ========== */
/* Self-referential struct (tree node) */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

/* Struct containing array of function pointers */
struct calculator {
    binary_op_t ops[4];
    char name[20];
};

/* Union with nested struct */
union nested_types {
    struct {
        int type;
        union {
            point_t pt;
            rectangle_t rect;
        } shape;
    } geometric;
    struct {
        int type;
        char* text;
        int length;
    } textual;
};

/* ========== Function Declarations ========== */
void use_all_types(void);
void opaque_use(void* ptr);

/* External function to prevent optimization */
extern void external_function(void*);

#endif /* TYPES_H */
