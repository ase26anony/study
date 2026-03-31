/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if building within GCC */
#ifdef IN_GCC
#include "gtype.h"
#else
/* Minimal GTY macro definition for standalone testing */
#define GTY(x) 
#endif

/* ==================== TYPE_SCALAR ==================== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */

/* ==================== TYPE_STRING ==================== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

/* ==================== TYPE_STRUCT ==================== */
struct plain_s {                        /* Plain C struct */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    char c;
    struct plain_s *next;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* GTY-tagged structs for garbage collection */
struct GTY(()) user_s {
    int id;
    string_t name;
    struct user_s *GTY((skip)) next;    /* Pointer with skip attribute */
};

struct GTY(()) complex_user {
    my_int value;
    struct user_s *GTY((chain_next("%s.next"))) chain;
    void *GTY((tag("0"))) anonymous;
};

/* Struct with nested GTY struct */
struct GTY(()) container {
    struct complex_user *item;
    int count;
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Plain union */
    int i;
    double d;
    void *p;
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    struct user_s *user_ptr;
    struct plain_s *plain_ptr;
    int int_val;
};

/* ==================== TYPE_POINTER ==================== */
/* Pointer typedefs */
typedef struct user_s *user_ptr_t;
typedef int *int_ptr_t;
typedef void (*func_ptr_t)(void);

/* Struct with various pointers */
struct GTY(()) pointer_heavy {
    int *scalar_ptr;                    /* Pointer to scalar */
    struct user_s **double_ptr;         /* Pointer to pointer */
    const char *const *const_ptr_ptr;   /* Const pointer to const pointer */
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with arrays */
struct GTY(()) array_struct {
    int fixed_array[10];                /* Fixed-size array */
    char string_array[5][20];           /* 2D array */
    struct user_s *ptr_array[5];        /* Array of pointers */
};

/* Variable-length array in GTY struct */
struct GTY(()) vla_container {
    int length;
    int data[1];                        /* Variable-length array idiom */
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_s *, const char *);

/* Struct with callback */
struct GTY(()) callback_container {
    simple_callback cb;
    complex_callback complex_cb;
    void (*inline_cb)(void);            /* Inline function pointer declaration */
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific struct definitions */
#ifdef GENERATOR_FILE
struct GTY(()) generator_specific {
    int gen_field;
    void *gen_ptr;
};
#endif

/* Conditional for front-end specific struct */
#ifdef GCC_JIT
struct GTY(()) jit_specific {
    void *jit_data;
    int jit_flags;
};
#endif

/* Another conditional struct */
#ifdef LANG_SPECIFIC
struct GTY(()) lang_struct {
    int lang_id;
    union tagged_union data;
};
#endif

/* ==================== RECURSIVE AND NESTED TYPES ==================== */
/* Mutual recursion */
struct GTY(()) node_a;
struct GTY(()) node_b;

struct GTY(()) node_a {
    int id;
    struct node_b *partner;
};

struct GTY(()) node_b {
    int id;
    struct node_a *partner;
    struct node_a *array_partners[3];
};

/* Complex nested type */
struct GTY(()) outer {
    struct {
        int inner_a;
        struct user_s *inner_ptr;
    } nested;
    
    union {
        int as_int;
        struct array_struct as_array;
    } variant;
    
    struct callback_container callbacks;
};

/* ==================== UNDEFINED TYPE HANDLING ==================== */
/* Forward declaration that might remain undefined */
struct forward_declared;

/* Struct using forward declared type */
struct GTY(()) uses_forward {
    struct forward_declared *future;    /* TYPE_UNDEFINED when forward not defined */
    int current;
};

/* ==================== TYPE_NONE ==================== */
/* This case (TYPE_NONE) should be unreachable in normal operation */
/* It represents an internal error state in gengtype */

#endif /* TEST_GENGTYPE_TYPES_H */
