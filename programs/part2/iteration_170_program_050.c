/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_GTYPE_DESC_H
#include "gtype-desc.h"
#endif

/* GTY macro definition for standalone testing */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef char my_char;                   /* Character scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

/* ========== TYPE_STRUCT ========== */
struct plain_s {                        /* Plain C struct (not GTY-tagged) */
    int a;
    double b;
};

struct another_plain {                  /* Another plain struct */
    struct plain_s *link;
    char name[32];
};

/* ========== TYPE_USER_STRUCT ========== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;
};

struct GTY(()) complex_user {           /* Another GTY-tagged struct */
    struct user_s *next;                /* Pointer to another GTY struct */
    struct user_s *prev;                /* Another pointer */
    int data;
};

/* ========== TYPE_UNION ========== */
union my_u {                            /* Plain union */
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *user_ptr;
    struct plain_s *plain_ptr;
    int value;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef struct plain_s **double_ptr_t;  /* Pointer to pointer */

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][32];           /* 2D array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*inline_cb)(struct user_s*); /* Inline function pointer */
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
/* Language-specific struct - only processed in generator context */
struct GTY(()) lang_specific_struct {
    struct user_s *base;
    int lang_data;
};
#endif

/* ========== Complex Nested Types ========== */
struct GTY(()) recursive_container {
    struct recursive_container *self;   /* Self-referential pointer */
    struct complex_user *complex;
    union tagged_union variant;
    
    /* Mixed array types */
    callback_fn callbacks[4];
    int matrix[3][3];
    
    /* Nested struct */
    struct {
        int nested_a;
        double nested_b;
    } inner;
};

/* ========== Undefined Type Forward Declarations ========== */
struct undefined_struct;                /* Forward declaration */
typedef struct undefined_struct *undef_ptr_t;

/* Later definition to resolve */
struct undefined_struct {
    int defined_now;
    struct user_s *link;
};

/* ========== Edge Cases ========== */
/* Struct with all type kinds */
struct GTY(()) all_types_struct {
    /* Scalar */
    my_int scalar_field;
    
    /* String */
    string_t string_field;
    
    /* Struct */
    struct plain_s plain_field;
    
    /* Pointer */
    struct user_s *pointer_field;
    
    /* Array */
    int array_field[8];
    
    /* Union */
    union my_u union_field;
    
    /* Callback */
    callback_fn callback_field;
    
    /* Nested anonymous struct */
    struct {
        int anonymous_a;
        struct user_s *anonymous_ptr;
    } anonymous;
};

/* Chain of pointers */
typedef struct GTY(()) chain_node {
    struct chain_node *next;
    struct chain_node *prev;
    void *data;
} chain_node_t;

/* Array of unions */
union GTY(()) multi_union {
    chain_node_t *node;
    struct all_types_struct *all_types;
    callback_fn func;
};

struct GTY(()) union_array_container {
    union multi_union unions[10];
    int count;
};

#endif /* TEST_GENGTYPE_TYPES_H`
