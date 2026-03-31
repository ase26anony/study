/* test_gengtype_types.h - Comprehensive type definitions for gengtype testing */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Forward declarations for recursive structures */
struct GTY(()) user_struct_t;
struct GTY(()) lang_struct_t;

/* ========== TYPE_SCALAR ========== */
typedef int my_int;                     /* Simple scalar typedef */
typedef unsigned long my_ulong;         /* Another scalar */
typedef double my_double;               /* Floating point scalar */
typedef char my_char;                   /* Character scalar */
typedef _Bool my_bool;                  /* Boolean scalar */

/* ========== TYPE_STRING ========== */
typedef const char *string_t;           /* String pointer type */
typedef char *mutable_string_t;         /* Mutable string pointer */

/* ========== TYPE_STRUCT ========== */
/* Plain C struct (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
    char field3;
};

/* Another plain struct */
struct another_plain {
    struct plain_struct *link;
    int counter;
};

/* ========== TYPE_USER_STRUCT ========== */
/* GTY-tagged structs for garbage collection */
struct GTY(()) user_struct_t {
    int id;                            /* Scalar field */
    string_t name;                     /* String field */
    struct plain_struct plain;         /* Nested plain struct */
    struct user_struct_t *next;        /* Recursive pointer */
    struct user_struct_t **prev_ptr;   /* Pointer to pointer */
};

/* GTY-tagged struct with array */
struct GTY(()) array_struct_t {
    int values[10];                    /* Fixed-size array */
    struct user_struct_t *items[5];    /* Array of pointers */
    double matrix[3][3];               /* 2D array */
};

/* GTY-tagged union container */
struct GTY(()) union_container_t {
    int type;
    union {
        int int_val;
        double double_val;
        struct user_struct_t *ptr_val;
    } GTY((desc("%0.type"))) data;    /* Tagged union */
};

/* ========== TYPE_UNION ========== */
/* Plain union (not GTY-tagged) */
union plain_union {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* Another plain union */
union data_union {
    long long_val;
    char char_val;
    float float_val;
};

/* ========== TYPE_POINTER ========== */
/* Pointer typedefs */
typedef struct user_struct_t *user_ptr_t;
typedef int *int_ptr_t;
typedef void (*func_ptr_t)(void);      /* Function pointer typedef */

/* GTY-tagged struct with various pointers */
struct GTY(()) pointer_struct_t {
    int *scalar_ptr;                   /* Pointer to scalar */
    string_t str_ptr;                  /* String pointer */
    struct user_struct_t *user_ptr;    /* Pointer to user struct */
    struct plain_struct *plain_ptr;    /* Pointer to plain struct */
    void *generic_ptr;                 /* Generic void pointer */
};

/* ========== TYPE_ARRAY ========== */
/* Array typedef */
typedef int int_array_t[100];

/* GTY-tagged struct with arrays */
struct GTY(()) complex_array_struct_t {
    int simple_array[20];              /* Simple array */
    struct user_struct_t *ptr_array[10]; /* Array of pointers */
    int multi_dim[2][3][4];            /* Multi-dimensional array */
    char string_array[5][50];          /* Array of strings */
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types (callbacks) */
typedef void (*simple_callback_t)(int);
typedef int (*complex_callback_t)(struct user_struct_t *, string_t);
typedef void (*void_callback_t)(void);

/* GTY-tagged struct with callback */
struct GTY(()) callback_struct_t {
    simple_callback_t handler;         /* Function pointer field */
    complex_callback_t processor;      /* Another function pointer */
    void (*inline_callback)(double);   /* Inline function pointer declaration */
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific struct definitions */
#ifdef GENERATOR_FILE
/* This struct should only be processed when GENERATOR_FILE is defined */
struct GTY(()) lang_struct_t {
    int lang_specific_field;
    struct user_struct_t *linked_item;
    #ifdef LANG_HOOKS
    void (*lang_hook)(void);
    #endif
};
#endif

/* Conditional struct for different contexts */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) conditional_struct_t {
    int context_id;
    #ifdef GENERATOR_FILE
    int generator_only_field;
    #endif
    #ifdef IN_GCC
    int gcc_only_field;
    #endif
};
#endif

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition (will be TYPE_UNDEFINED) */
struct undefined_struct;

/* Pointer to undefined struct */
typedef struct undefined_struct *undefined_ptr_t;

/* GTY-tagged struct referencing undefined type */
struct GTY(()) references_undefined_t {
    int valid_field;
    struct undefined_struct *undefined_ptr;  /* Pointer to undefined type */
};

/* ========== Complex Nested Example ========== */
/* Master struct that includes many different types */
struct GTY(()) master_struct_t {
    /* Scalars */
    my_int int_field;
    my_double double_field;
    
    /* Strings */
    string_t name_field;
    mutable_string_t buffer_field;
    
    /* Structs */
    struct plain_struct plain_field;
    struct user_struct_t user_field;
    
    /* Pointers */
    struct master_struct_t *self_ptr;
    struct user_struct_t **double_ptr;
    
    /* Arrays */
    int number_array[15];
    struct user_struct_t *struct_array[8];
    
    /* Callbacks */
    simple_callback_t callback_field;
    
    /* Union */
    union plain_union union_field;
    
    /* Language-specific (conditional) */
    #ifdef GENERATOR_FILE
    struct lang_struct_t *lang_field;
    #endif
    
    /* Reference to undefined type */
    undefined_ptr_t undefined_field;
};

/* ========== Additional Type Combinations ========== */

/* Struct containing array of callbacks */
struct GTY(()) callback_array_struct_t {
    simple_callback_t handlers[5];
    complex_callback_t processors[3];
};

/* Union with GTY-tagged pointer */
union GTY(()) tagged_union_t {
    int tag;
    struct {
        int type;
        struct user_struct_t * GTY((tag("0.type"))) data;
    } s;
};

/* Typedef chain */
typedef struct user_struct_t base_t;
typedef base_t *base_ptr_t;
typedef base_ptr_t *base_double_ptr_t;

/* Const pointer typedef */
typedef const struct user_struct_t *const_user_ptr_t;

/* Volatile pointer typedef */
typedef volatile int *volatile_int_ptr_t;

#endif /* TEST_GENGTYPE_TYPES_H */
