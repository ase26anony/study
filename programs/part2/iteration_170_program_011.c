/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* For TYPE_LANG_STRUCT testing */
#ifdef GENERATOR_FILE
#define GTY_LANG(x) x
#else
#define GTY_LANG(x)
#endif

/* Basic scalar types - TYPE_SCALAR */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;
typedef char my_char;

/* String type - TYPE_STRING */
typedef const char *string_t;
typedef char *mutable_string_t;

/* Plain struct - TYPE_STRUCT */
struct plain_s {
    int a;
    double b;
};

/* Another plain struct */
struct another_plain {
    struct plain_s *link;
    int count;
};

/* Union type - TYPE_UNION */
union my_u {
    int i;
    void *p;
    double d;
};

/* Nested union */
union complex_u {
    struct {
        int type;
        union my_u data;
    } header;
    long long combined;
};

/* Callback type - TYPE_CALLBACK */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, void *);
typedef void (*void_callback)(void);

/* Array type definitions */
typedef int int_array[10];
typedef struct plain_s struct_array[5];

/* Pointer types - TYPE_POINTER */
typedef struct plain_s *plain_ptr_t;
typedef union my_u *union_ptr_t;
typedef void (*callback_ptr_t)(int);

/* GTY-tagged structs - TYPE_USER_STRUCT */

/* Simple GTY struct */
struct GTY(()) user_s {
    struct plain_s *plain_link;  /* Pointer field */
    int id;
    char name[32];               /* Array field */
};

/* GTY struct with array of pointers */
struct GTY(()) container_s {
    struct user_s *items[20];    /* Array of pointers */
    int count;
    simple_callback notify;      /* Callback field */
};

/* Recursive GTY struct */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;   /* Skip annotation */
    struct tree_node *right;
    struct tree_node *parent;
};

/* GTY struct with union */
struct GTY(()) variant_s {
    int type;
    union {
        int int_val;
        double double_val;
        struct user_s *GTY((tag("1"))) user_ptr;
        string_t str_val;
    } GTY((desc("type"))) data;
};

/* GTY struct with callback */
struct GTY(()) event_handler {
    const char *name;
    complex_callback handler;
    void *user_data;
};

/* Language-specific struct - TYPE_LANG_STRUCT */
#ifdef GENERATOR_FILE
struct GTY_LANG(()) lang_specific {
    int magic;
    void *data;
};
#endif

/* Another language struct variant */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) gcc_lang_struct {
    int code;
    union my_u value;
    struct gcc_lang_struct *next;
};
#endif

/* Complex nested type example */
struct GTY(()) complex_nested {
    /* Array of structs containing pointers */
    struct {
        struct user_s *user;
        int_array scores;
    } entries[8];
    
    /* Union with different pointer types */
    union {
        struct container_s *container;
        struct variant_s *variant;
        callback_ptr_t callback;
    } current;
    
    /* Callback array */
    void_callback handlers[4];
    
    /* Pointer to plain struct */
    struct plain_s *plain_data;
};

/* Typedef for GTY pointer */
typedef struct user_s * GTY((user)) user_ptr;

/* GTY union */
union GTY(()) gty_union {
    int ival;
    double dval;
    struct user_s *uptr;
    string_t sval;
};

/* Struct with multiple array types */
struct GTY(()) array_types {
    int scalar_array[20];                /* Simple array */
    struct plain_s *pointer_array[15];   /* Array of pointers */
    int (*callback_array[5])(void);      /* Array of callbacks */
    int multi_dim[3][4][5];              /* Multi-dimensional */
};

/* Edge case: struct with no fields */
struct GTY(()) empty_struct {
    /* Intentionally empty */
};

/* Struct with only arrays */
struct GTY(()) arrays_only {
    char buffer[256];
    int matrix[10][10];
    void *pointers[50];
};

#endif /* TEST_GENGTYPE_TYPES_H */
