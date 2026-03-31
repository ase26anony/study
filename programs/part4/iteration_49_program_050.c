#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <complex.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;

/* Scalar types (TYPE_SCALAR) */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    int outer;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

struct array_member_struct {
    int ids[10];
    char name[50];
    struct nested_struct nested;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct tagged_struct {
    enum { TAG_A, TAG_B, TAG_C } tag;
    union {
        int int_val;
        double dbl_val;
        char* str_val;
    } data;
} tagged_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

union nested_union {
    struct {
        int type;
        union simple_union data;
    } tagged;
    long long raw;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][5];
typedef int int_array_3d[3][3][3];
typedef struct simple_struct struct_array[5];
typedef union simple_union union_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef int (*callback_with_args)(int, char*, double);
typedef void (*callback_complex)(struct simple_struct*, union simple_union*, int_array_10);
typedef int (*callback_returning_ptr)(void);
typedef void (*callback_varargs)(const char*, ...);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Anonymous struct/union */
struct container {
    int id;
    struct {
        int x;
        int y;
    } point;
    union {
        int a;
        float b;
    } value;
};

/* Linked list structure for type dependencies */
struct list_node {
    void* data;
    struct list_node* next;
    struct list_node* prev;
};

/* Tree structure */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
};

/* Function declarations */
void use_all_types(void);
extern void external_function(void*);

#endif /* TYPE_DEFS_H */
