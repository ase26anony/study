/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) x
#endif

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef char my_char;                   /* Character scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string */

/* ========== TYPE_STRUCT (plain) ========== */
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
    struct plain_s *p;                  /* Pointer to plain struct */
    my_int value;
};

struct GTY(()) tree_node {
    int code;
    struct tree_node *GTY((skip)) left;  /* Skip this field for GC */
    struct tree_node *right;
    string_t name;
};

struct GTY(()) complex_user {
    struct user_s *user_ptr;
    struct tree_node *tree_root;
    int flags;
};

/* ========== TYPE_UNION ========== */
union my_u {
    int i;
    void *p;
    double d;
};

union GTY(()) tagged_union {
    struct user_s *GTY((tag("0"))) as_user;
    struct tree_node *GTY((tag("1"))) as_tree;
    int GTY((default)) as_int;
};

/* ========== TYPE_POINTER ========== */
typedef struct user_s *user_ptr_t;
typedef struct tree_node **tree_dbl_ptr_t;

struct GTY(()) pointer_container {
    void *raw_ptr;                      /* TYPE_POINTER */
    struct user_s *user_ptr;            /* TYPE_POINTER to user struct */
    struct plain_s *plain_ptr;          /* TYPE_POINTER to plain struct */
};

/* ========== TYPE_ARRAY ========== */
struct GTY(()) array_container {
    int fixed_arr[10];                  /* Fixed-size array */
    struct user_s *ptr_arr[5];          /* Array of pointers */
    char str_arr[3][20];                /* 2D array */
};

/* Variable-length array in GTY struct */
struct GTY(()) varray_container {
    int length;
    int elements[1];                    /* Variable length array */
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*callback_fn)(int);       /* Function pointer typedef */
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) callback_container {
    callback_fn handler;                /* Callback field */
    compare_fn comparator;
    void (*inline_cb)(struct user_s *); /* Inline callback declaration */
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs using conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) generator_specific {
    int gen_field;
    void *gen_ptr;
};
#endif

#ifdef LANG_HOOKS
struct GTY(()) lang_hook_struct {
    int lang_id;
    struct tree_node *lang_tree;
};
#endif

/* Nested anonymous struct/unions for complexity */
struct GTY(()) nested_example {
    union {
        int as_int;
        struct {
            short a;
            short b;
        } as_pair;
    } data;
    
    struct {
        callback_fn cb;
        int state;
    } handler;
};

/* Recursive type pattern */
struct GTY(()) recursive_node {
    int value;
    struct recursive_node *next;        /* Recursive pointer */
    struct recursive_node *children[4]; /* Array of recursive pointers */
};

/* Mixed container with all types */
struct GTY(()) mega_container {
    /* Scalars */
    my_int id;
    my_double weight;
    
    /* Strings */
    string_t name;
    mutable_string_t buffer;
    
    /* Structs */
    struct plain_s plain;
    struct user_s *user;
    
    /* Unions */
    union my_u variant;
    
    /* Pointers */
    void **ptr_table;
    
    /* Arrays */
    callback_fn handlers[8];
    struct tree_node *node_grid[3][3];
    
    /* Callback */
    compare_fn sorter;
    
    /* Language-specific (conditional) */
#ifdef GENERATOR_FILE
    struct generator_specific *gen_spec;
#endif
};

/* Undefined forward declaration (will be TYPE_UNDEFINED initially) */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_t;

/* Later definition to resolve */
struct undefined_struct {
    int defined_now;
};

#endif /* TEST_GENGTYPE_TYPES_H */
