/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (handled as scalar) */
enum color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_UNDEFINED: Type without GTY marker, referenced by annotated types */
struct undefined_struct {
    int id;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;                     /* TYPE_SCALAR */
    char *name;                 /* TYPE_POINTER -> TYPE_STRING */
    struct undefined_struct *ref; /* TYPE_POINTER -> TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;            /* TYPE_POINTER -> TYPE_SCALAR */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;                /* TYPE_SCALAR */
    double double_val;          /* TYPE_SCALAR */
    char *string_val;           /* TYPE_POINTER -> TYPE_STRING */
    struct basic_struct *struct_ptr; /* TYPE_POINTER -> TYPE_STRUCT */
};

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];    /* TYPE_ARRAY of TYPE_SCALAR */
typedef struct basic_struct *struct_ptr_array_t[5]; /* TYPE_ARRAY of TYPE_POINTER */

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;                 /* TYPE_SCALAR */
    char *data;                 /* TYPE_POINTER -> TYPE_STRING */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *data, int param); /* TYPE_CALLBACK */

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func_t handler;    /* TYPE_CALLBACK */
    void *user_data;            /* TYPE_POINTER -> TYPE_SCALAR */
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;      /* TYPE_POINTER to TYPE_STRUCT */
typedef union data_union *union_ptr_t;          /* TYPE_POINTER to TYPE_UNION */
typedef int_array_t *array_ptr_t;               /* TYPE_POINTER to TYPE_ARRAY */

/* Linked list for deep traversal */
struct GTY(()) list_node {
    int value;                  /* TYPE_SCALAR */
    struct list_node *next;     /* TYPE_POINTER -> TYPE_STRUCT (self-reference) */
    struct list_node *prev;     /* TYPE_POINTER -> TYPE_STRUCT */
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct basic_struct base;   /* TYPE_STRUCT */
    union data_union data;      /* TYPE_UNION */
    int_array_t numbers;        /* TYPE_ARRAY */
    struct_ptr_array_t pointers; /* TYPE_ARRAY of TYPE_POINTER */
    struct gcc_string str;      /* TYPE_STRUCT (string-like) */
    callback_func_t callback;   /* TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    /* Basic types */
    struct basic_struct *basic_ptr;     /* TYPE_POINTER -> TYPE_STRUCT */
    struct user_struct *user_ptr;       /* TYPE_POINTER -> TYPE_USER_STRUCT */
    
    /* Containers */
    union data_union *union_ptr;        /* TYPE_POINTER -> TYPE_UNION */
    struct complex_nested *nested_ptr;  /* TYPE_POINTER -> TYPE_STRUCT */
    
    /* Strings and callbacks */
    struct gcc_string *string_ptr;      /* TYPE_POINTER -> TYPE_STRUCT */
    struct callback_container *callback_ptr; /* TYPE_POINTER -> TYPE_STRUCT */
    
    /* Arrays */
    int_array_t direct_array;           /* TYPE_ARRAY */
    struct_ptr_array_t struct_array;    /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Linked structure */
    struct list_node *list_head;        /* TYPE_POINTER -> TYPE_STRUCT */
    
    /* Scalars */
    scalar_int_t int_scalar;            /* TYPE_SCALAR */
    scalar_double_t double_scalar;      /* TYPE_SCALAR */
    enum color color_enum;              /* TYPE_SCALAR (enum) */
    
    /* Undefined reference */
    struct undefined_struct *undefined_ptr; /* TYPE_POINTER -> TYPE_UNDEFINED */
    
    /* Various pointers */
    struct_ptr_t struct_ptr;            /* TYPE_POINTER -> TYPE_STRUCT */
    union_ptr_t union_ptr_alias;        /* TYPE_POINTER -> TYPE_UNION */
    array_ptr_t array_ptr;              /* TYPE_POINTER -> TYPE_ARRAY */
    
    /* Function pointer */
    callback_func_t direct_callback;    /* TYPE_CALLBACK */
};

/* Global root variable */
extern struct root_container GTY(()) global_root;

#endif /* GTY_TEST_H */
