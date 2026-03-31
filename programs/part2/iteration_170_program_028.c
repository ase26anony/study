/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers for GTY macro */
#ifdef GENERATOR_FILE
#include "gtype-desc.h"
#else
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
struct plain_s {                        /* Untagged struct */
    int a;
    double b;
};

struct another_plain {                  /* Another untagged struct */
    char c;
    struct plain_s *next;
};

/* ==================== TYPE_USER_STRUCT ==================== */
struct GTY(()) user_s {                 /* GTY-tagged struct */
    int id;
    string_t name;
    struct user_s *GTY((skip)) next;    /* Pointer with skip attribute */
};

struct GTY(()) complex_user {           /* Another GTY-tagged struct */
    my_int value;
    struct user_s *GTY((chain_next("%s.next"))) chain;
    void *GTY((tag("0"))) anonymous;    /* Tagged pointer */
};

/* ==================== TYPE_UNION ==================== */
union my_u {                            /* Simple union */
    int i;
    double d;
    void *p;
};

union GTY(()) tagged_union {            /* GTY-tagged union */
    struct user_s *GTY((desc("1"))) usr;
    struct plain_s *plain;
    int scalar;
};

/* ==================== TYPE_POINTER ==================== */
typedef struct user_s *user_ptr_t;      /* Pointer typedef */
typedef int *int_ptr_t;                 /* Another pointer typedef */

/* ==================== TYPE_ARRAY ==================== */
struct GTY(()) array_container {
    int fixed_arr[10];                  /* Fixed-size array */
    struct user_s *GTY((length("len"))) varray[5]; /* Variable-length array marker */
    int len;
};

/* ==================== TYPE_CALLBACK ==================== */
typedef void (*callback_fn)(int, void*); /* Function pointer typedef */
typedef int (*compare_fn)(const void*, const void*);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
#ifdef GENERATOR_FILE
/* This struct should only be visible to generator */
struct GTY(()) lang_specific {
    int generator_only_field;
    struct user_s *link;
};
#endif

/* ==================== NESTED/RECURSIVE PATTERNS ==================== */

/* Recursive structure */
struct GTY(()) tree_node {
    int type;
    string_t name;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union my_u data;
};

/* Container with multiple type kinds */
struct GTY(()) container {
    /* Scalar fields */
    my_int count;
    my_double average;
    
    /* String field */
    string_t description;
    
    /* Struct fields */
    struct plain_s base;
    
    /* User struct pointers */
    struct user_s *owner;
    struct complex_user *complex;
    
    /* Union field */
    union tagged_union variant;
    
    /* Array fields */
    int scores[5];
    struct user_s *members[3];
    
    /* Pointer array */
    void *pointers[8];
    
    /* Callback field */
    callback_fn on_update;
    
    /* Nested container */
    struct container *GTY((skip)) next;
    
#ifdef GENERATOR_FILE
    /* Language-specific field */
    struct lang_specific *lang_data;
#endif
};

/* Forward declarations to test TYPE_UNDEFINED handling */
struct forward_declared;                /* Will be TYPE_UNDEFINED initially */
typedef struct forward_declared *fwd_ptr_t;

/* Later definition */
struct GTY(()) forward_declared {
    int defined_now;
    fwd_ptr_t self_ref;                 /* Self-referential pointer */
};

/* Mixed declaration order test */
struct GTY(()) node_a;
struct GTY(()) node_b;

struct GTY(()) node_a {
    struct node_b *partner;
    int value_a;
};

struct GTY(()) node_b {
    struct node_a *partner;
    int value_b;
};

/* Test for TYPE_NONE - this should not appear in normal parsing */
/* TYPE_NONE is for internal error cases, not user-defined types */

#endif /* TEST_GENGTYPE_TYPES_H */
