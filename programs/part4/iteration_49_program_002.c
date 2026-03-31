#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>
#include <stddef.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* Scalar types (TYPE_SCALAR) */
typedef char my_char;
typedef short my_short;
typedef int my_int;
typedef long my_long;
typedef long long my_longlong;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128_t;

/* String type (TYPE_STRING) */
typedef const char* my_string_t;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct basic_struct {
    int x;
    double y;
    char z;
};

typedef struct {
    int a;
    float b;
    char c[10];
} anon_struct_t;

struct nested_struct {
    struct basic_struct inner;
    anon_struct_t another;
};

/* Union types (TYPE_UNION) */
union basic_union {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long l;
    double d;
    void *p;
} anon_union_t;

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct basic_struct* struct_ptr_t;
typedef union basic_union* union_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef int int_matrix_3x3_t[3][3];
typedef int int_cube_2x2x2_t[2][2][2];
typedef struct basic_struct struct_array_5_t[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(int, char*, ...);
typedef int (*binary_op_t)(int, int);
typedef void (*struct_callback_t)(struct basic_struct*);

/* Language struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list my_va_list_t;

/* Vector types */
typedef int __attribute__((vector_size(16))) int_vec4_t;
typedef float __attribute__((vector_size(32))) float_vec8_t;

/* Packed and aligned structs */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* unnamed bitfield */
    unsigned int value : 20;
};

/* Anonymous struct/union within struct */
struct container {
    int tag;
    union {
        struct {
            int x;
            float y;
        } point;
        struct {
            char name[20];
            int id;
        } info;
    } data;
};

/* Linked list structures for type dependencies */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
};

struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
};

/* Function pointer arrays */
typedef int (*op_func_t)(int, int);
extern op_func_t operations[10];

/* External declarations */
extern struct undefined_struct* get_undefined_ptr(void);
extern void use_all_types(void);

#endif /* TYPES_H */
