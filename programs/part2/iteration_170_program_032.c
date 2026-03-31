/* gengtype_test.h - Comprehensive type definitions for gengtype coverage testing */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#ifdef GENERATOR_FILE
/* This macro indicates we're being processed by gengtype */
#endif

/* Include gtype-desc.h for GTY macro definition */
#include "gtype-desc.h"

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string pointer */

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer type */
typedef int (*compare_fn)(const void *, const void *);  /* Another callback */

/* ========== TYPE_STRUCT (untagged) ========== */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    char c;
    long d;
};

/* ========== TYPE_UNION ========== */
union my_u {
    int i;
    void *p;
    double d;
};

union tagged_union {
    long tag;
    struct plain_s *data;
};

/* ========== TYPE_USER_STRUCT (GTY-tagged) ========== */
struct GTY(()) user_s {
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int count;                       /* Scalar type */
    string_t name;                      /* String type */
};

struct GTY(()) complex_user {
    struct user_s *next;                /* Pointer to another GTY struct */
    struct user_s *prev;                /* Recursive pointer */
    union my_u data;                    /* Union field */
    callback_fn handler;                /* Callback field */
};

/* ========== TYPE_POINTER ========== */
/* These will be counted when encountered in struct fields */
/* Also explicit pointer typedefs: */
typedef struct user_s *user_ptr_t;
typedef void *generic_ptr_t;

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct user_s *ptr_array[5];        /* Array of pointers */
    char string_array[3][50];           /* 2D array */
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific {
    int lang_field;
    struct user_s *lang_ptr;
};
#endif

/* ========== More complex nested structures ========== */
struct GTY(()) tree_node {
    int code;
    union {
        long intval;
        double realval;
        string_t strval;
    } GTY((desc("%1.code"))) u;
    struct tree_node *children[2];
};

/* Union with GTY marker */
union GTY(()) gty_union {
    struct user_s *us;
    struct array_container *ac;
};

/* Struct with array of callbacks */
struct GTY(()) callback_container {
    callback_fn handlers[4];
    compare_fn comparators[2];
};

/* Self-referential structure */
struct GTY(()) linked_node {
    int value;
    struct linked_node *GTY((skip)) next;  /* Skip this for GC tracing */
    struct linked_node *prev;
};

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
    int type;
#ifdef GENERATOR_FILE
    struct lang_specific *lang_data;
#else
    void *other_data;
#endif
    union my_u current;
};

/* ========== Mixed declarations ========== */
typedef struct GTY(()) {
    int anonymous_field;
    struct user_s *link;
} anonymous_struct_t;

/* External declaration to test TYPE_UNDEFINED? */
/* Note: TYPE_UNDEFINED might be triggered by forward declarations */
struct undefined_struct;  /* Forward declaration */

struct GTY(()) uses_undefined {
    struct undefined_struct *future;  /* Pointer to undefined type */
    int present;
};

#endif /* GTY_TEST_H */
