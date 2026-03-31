/* test_gengtype_types.h - Comprehensive type definitions for gengtype testing */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Forward declarations to create type dependencies */
struct plain_struct;
struct user_struct;
union test_union;

/* TYPE_SCALAR: Basic scalar typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;
typedef long long my_longlong;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int);
typedef int (*compare_fn)(const void *, const void *);
typedef void (*traverse_fn)(void *);

/* TYPE_POINTER: Pointer typedefs */
typedef struct plain_struct *plain_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_STRUCT: Plain C struct (not GTY-tagged) */
struct plain_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_UNION: Union definition */
union test_union {
    int as_int;
    double as_double;
    void *as_pointer;
    char as_string[16];
};

/* TYPE_ARRAY: Array type within struct */
struct array_container {
    int numbers[10];
    char *strings[5];
    struct plain_struct structs[3];
};

/* GTY-tagged types for garbage collection */

/* TYPE_USER_STRUCT: GTY-tagged struct */
struct GTY(()) user_struct {
    int id;
    string_t name;                     /* TYPE_STRING */
    struct plain_struct *plain_ref;    /* TYPE_POINTER to TYPE_STRUCT */
    struct user_struct *next;          /* TYPE_POINTER to TYPE_USER_STRUCT (recursive) */
    callback_fn handler;               /* TYPE_CALLBACK */
    int scores[20];                    /* TYPE_ARRAY of scalar */
};

/* Another GTY-tagged struct with complex nesting */
struct GTY(()) complex_struct {
    struct user_struct *users[10];     /* TYPE_ARRAY of TYPE_POINTER to TYPE_USER_STRUCT */
    union test_union data;             /* TYPE_UNION */
    compare_fn comparator;             /* TYPE_CALLBACK */
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int depth;
        struct complex_struct *parent; /* Recursive pointer */
    } node;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    void *generator_data;
};
#endif

/* Union containing GTY-tagged pointer */
union GTY(()) tagged_union {
    struct user_struct *user_ptr;
    struct complex_struct *complex_ptr;
    int tag;
};

/* Struct with multiple array types */
struct GTY(()) multi_array_struct {
    /* Fixed-size array of scalars */
    int matrix[4][4];
    
    /* Array of pointers */
    struct user_struct *ptr_array[8];
    
    /* Array of arrays */
    char string_table[16][64];
    
    /* Pointer to array */
    int (*dynamic_array)[10];
};

/* Callback type used in GTY struct */
typedef void GTY((user)) (*user_callback)(struct user_struct *);

/* Struct using the callback type */
struct GTY(()) callback_container {
    user_callback pre_process;
    user_callback post_process;
    void *user_data;
};

/* Linked list example with multiple type kinds */
struct GTY(()) list_node {
    int type;                          /* TYPE_SCALAR */
    union {
        int int_value;                 /* TYPE_SCALAR */
        string_t str_value;            /* TYPE_STRING */
        struct plain_struct *plain;    /* TYPE_POINTER to TYPE_STRUCT */
    } data;                            /* TYPE_UNION */
    struct list_node *next;            /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct list_node *prev;            /* TYPE_POINTER to TYPE_USER_STRUCT */
};

/* Undefined type reference (forward declared, will be TYPE_UNDEFINED initially) */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_t;

/* Struct with undefined type pointer */
struct GTY(()) has_undefined {
    undefined_ptr_t undefined_field;   /* TYPE_POINTER to TYPE_UNDEFINED */
    int defined_field;
};

#endif /* TEST_GENGTYPE_TYPES_H */
