/* type_defs.h - Comprehensive type definitions to stress GCC's type system */

#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* ========== UNDEFINED/INCOMPLETE TYPES ========== */
struct forward_declared_struct;      /* TYPE_UNDEFINED */
union forward_declared_union;        /* TYPE_UNDEFINED */

/* ========== SCALAR TYPES ========== */
typedef int scalar_int_t;            /* TYPE_SCALAR */
typedef char scalar_char_t;          /* TYPE_SCALAR */
typedef short scalar_short_t;        /* TYPE_SCALAR */
typedef long scalar_long_t;          /* TYPE_SCALAR */
typedef float scalar_float_t;        /* TYPE_SCALAR */
typedef double scalar_double_t;      /* TYPE_SCALAR */
typedef _Bool scalar_bool_t;         /* TYPE_SCALAR */
typedef _Complex float complex_float_t;  /* TYPE_SCALAR */
typedef _Complex double complex_double_t; /* TYPE_SCALAR */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;           /* TYPE_SCALAR */
typedef unsigned __int128 uint128_t; /* TYPE_SCALAR */
#endif

/* ========== STRING TYPES ========== */
typedef char* string_ptr_t;          /* TYPE_STRING */
typedef const char* const_string_ptr_t; /* TYPE_STRING */

/* ========== STRUCT TYPES ========== */
struct simple_struct {               /* TYPE_STRUCT */
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {               /* TYPE_STRUCT with nested types */
    struct simple_struct inner;
    struct nested_struct* next;      /* Pointer to self */
    volatile int counter;
};

struct bitfield_struct {             /* TYPE_STRUCT with bitfields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int : 16;               /* Padding */
} __attribute__((aligned(8)));

/* Anonymous struct within struct */
struct container_struct {            /* TYPE_STRUCT */
    int id;
    struct {                         /* Anonymous struct */
        float x, y, z;
    } position;
    union {                          /* Anonymous union */
        int int_val;
        float float_val;
    } data;
};

/* ========== USER STRUCT TYPES ========== */
/* These would typically be defined via GTY(()) macros in GCC context */
/* We'll simulate with typedef struct combinations */
typedef struct simple_struct user_struct_t;      /* TYPE_USER_STRUCT */
typedef struct nested_struct user_nested_t;      /* TYPE_USER_STRUCT */

/* ========== UNION TYPES ========== */
union simple_union {                 /* TYPE_UNION */
    int as_int;
    float as_float;
    char as_char[4];
};

union tagged_union {                 /* TYPE_UNION with tag */
    enum { INT, FLOAT, STRING } tag;
    struct {
        int int_value;
    } i;
    struct {
        float float_value;
    } f;
    struct {
        char* string_value;
    } s;
};

/* ========== POINTER TYPES ========== */
typedef int* int_ptr_t;              /* TYPE_POINTER */
typedef int** int_ptr_ptr_t;         /* TYPE_POINTER to pointer */
typedef int*** int_ptr_ptr_ptr_t;    /* Triple pointer */
typedef void (*void_func_ptr_t)(void); /* Function pointer */

/* ========== ARRAY TYPES ========== */
typedef int int_array_10_t[10];      /* TYPE_ARRAY */
typedef int multi_array_t[5][10][15]; /* Multi-dimensional */
typedef char* string_array_t[20];    /* Array of pointers */

/* ========== CALLBACK TYPES ========== */
typedef int (*binary_op_t)(int, int); /* TYPE_CALLBACK */
typedef void (*callback_t)(void* data, int result);
typedef int (*variadic_func_t)(int count, ...);

/* ========== LANGUAGE STRUCT TYPES ========== */
/* Simulating GCC internal lang_struct types */
struct lang_struct_base {
    int lang_specific;
    void* lang_data;
};

/* ========== COMPLEX TYPE RELATIONSHIPS ========== */
/* Linked list node with multiple pointer types */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
    void (*print)(struct list_node*);
};

/* Tree node with different child types */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    union {
        int int_data;
        double double_data;
    } extra;
};

/* Array of function pointers */
typedef int (*math_ops_t[5])(int, int);

/* Struct with flexible array member */
struct flex_array_struct {
    int count;
    double data[];  /* Flexible array member */
};

/* Vector types if supported */
#ifdef __GNUC__
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));
#endif

/* ========== FUNCTION PROTOTYPES ========== */
void use_all_types(void);
void opaque_use(void* ptr);

#endif /* TYPE_DEFS_H */
