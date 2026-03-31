/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_GTYPE_DESC_H
#include "gtype-desc.h"
#endif

/* For language-specific structs */
#ifdef GENERATOR_FILE
#define LANG_STRUCT_MARKER
#else
#define LANG_STRUCT_MARKER
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                    /* Simple scalar typedef */
typedef unsigned long my_ulong;        /* Another scalar */
typedef double my_float;               /* Floating point scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;          /* String pointer type */
typedef char *mutable_string_t;        /* Mutable string */

/* ========== TYPE_STRUCT (untagged) ========== */
struct plain_s {
    int a;
    double b;
};

struct another_plain {
    struct plain_s *link;
    int count;
};

/* ========== TYPE_USER_STRUCT (GTY-tagged) ========== */
struct GTY(()) user_s {
    struct plain_s *plain_ptr;         /* Pointer to untagged struct */
    int scalar_field;
    double float_field;
};

struct GTY(()) complex_user_s {
    struct user_s *next;               /* Pointer to another GTY struct */
    struct user_s *prev;               /* Another pointer */
    int id;
};

/* ========== TYPE_UNION ========== */
union my_u {
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {
    struct user_s *user_ptr;
    struct complex_user_s *complex_ptr;
    int value;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;     /* Pointer typedef */
typedef struct complex_user_s **double_ptr_t;  /* Pointer to pointer */

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_array[10];               /* Fixed-size array */
    struct user_s *ptr_array[5];       /* Array of pointers */
    double multi_dim[3][4];            /* Multi-dimensional array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);      /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;               /* Callback field in GTY struct */
    compare_fn comparator;
    void (*another_callback)(struct user_s *);
};

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_s {
    int generator_only_field;
    struct user_s *linked_data;
};
#else
struct GTY(()) lang_specific_s {
    int normal_field;
    struct user_s *linked_data;
};
#endif

/* ========== Complex nested example ========== */
struct GTY(()) nested_example {
    /* Contains multiple type kinds */
    int scalar;                        /* TYPE_SCALAR */
    string_t name;                     /* TYPE_STRING */
    struct plain_s plain;              /* TYPE_STRUCT */
    union my_u data;                   /* TYPE_UNION */
    struct user_s *user_ptr;           /* TYPE_POINTER */
    callback_fn handlers[3];           /* TYPE_ARRAY of TYPE_CALLBACK */
    struct complex_user_s *complex;    /* Another TYPE_POINTER */
    
    /* Self-referential pointer */
    struct nested_example *self;
    
    /* Array of pointers to different types */
    void *ptr_array[8];                /* TYPE_ARRAY of TYPE_POINTER */
};

/* ========== More edge cases ========== */
/* Pointer to array */
typedef int (*array_ptr_t)[10];

/* Const pointer in GTY struct */
struct GTY(()) const_ptr_example {
    const struct user_s *readonly_ptr;
    volatile int volatile_field;
};

/* Anonymous struct/union */
struct GTY(()) anonymous_container {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } data;
};

/* Forward declaration that creates TYPE_UNDEFINED initially */
struct forward_declared_s;

struct GTY(()) uses_forward {
    struct forward_declared_s *future;  /* Will be TYPE_UNDEFINED initially */
    int current;
};

/* Later definition */
struct GTY(()) forward_declared_s {
    int defined_now;
    struct uses_forward *back_ref;
};

#endif /* TEST_GENGTYPE_TYPES_H */
