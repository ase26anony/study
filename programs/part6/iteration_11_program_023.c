/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_ARRAY: Array typedef */
typedef int int_array_t[10];
typedef char char_array_t[256];

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*callback_func_t)(void *data, int param);
typedef void (*simple_callback_t)(void);

/* Non-annotated struct (may become TYPE_UNDEFINED when referenced) */
struct unannotated_struct {
    int x;
    double y;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    int id;                     /* TYPE_SCALAR */
    char *name;                 /* TYPE_STRING */
    double value;               /* TYPE_SCALAR */
};

/* TYPE_STRUCT: More complex struct with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *base;  /* TYPE_POINTER */
    int_array_t numbers;        /* TYPE_ARRAY */
    callback_func_t callback;   /* TYPE_CALLBACK */
    enum color color;           /* TYPE_SCALAR */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;                /* TYPE_SCALAR */
    double double_val;          /* TYPE_SCALAR */
    char *string_val;           /* TYPE_STRING */
    void *ptr_val;              /* TYPE_POINTER */
};

/* TYPE_POINTER: Pointer-only typedef */
typedef struct basic_struct *basic_ptr_t;

/* TYPE_ARRAY: Array of pointers */
typedef struct complex_struct *complex_ptr_array_t[5];

/* Linked list structure for chained traversal */
struct GTY(()) list_node {
    int data;                   /* TYPE_SCALAR */
    struct list_node *next;     /* TYPE_POINTER */
    struct list_node *prev;     /* TYPE_POINTER */
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;                 /* TYPE_SCALAR */
    char *data;                 /* TYPE_STRING */
    char *allocated_data;       /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;
    struct unannotated_struct *unannotated; /* May trigger TYPE_UNDEFINED */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    union data_union lang_data; /* TYPE_UNION */
    struct lang_struct *next_lang; /* TYPE_POINTER */
};

/* Container with array of structs */
struct GTY(()) container {
    struct basic_struct items[4];           /* TYPE_ARRAY of structs */
    struct complex_struct *ptr_items[3];    /* TYPE_ARRAY of pointers */
    union data_union optional_data;         /* TYPE_UNION */
};

/* Root structure containing pointers to everything */
struct GTY(()) root_struct {
    /* Various pointer types */
    struct basic_struct *basic_ptr;         /* TYPE_POINTER */
    struct complex_struct *complex_ptr;     /* TYPE_POINTER */
    struct list_node *list_head;            /* TYPE_POINTER */
    struct gcc_string *string_obj;          /* TYPE_POINTER */
    struct user_struct *user_obj;           /* TYPE_POINTER */
    struct lang_struct *lang_obj;           /* TYPE_POINTER */
    struct container *container_obj;        /* TYPE_POINTER */
    
    /* Direct members */
    union data_union direct_union;          /* TYPE_UNION */
    callback_func_t root_callback;          /* TYPE_CALLBACK */
    int_array_t root_array;                 /* TYPE_ARRAY */
    struct gcc_string embedded_string;      /* TYPE_STRING */
    
    /* Array of various types */
    struct basic_struct basic_array[2];     /* TYPE_ARRAY */
    struct list_node *node_array[3];        /* TYPE_ARRAY of pointers */
    
    /* For undefined type testing */
    struct unannotated_struct *undef_ptr;   /* TYPE_POINTER to non-GTY */
};

/* Additional pointer typedefs */
typedef struct root_struct *root_ptr_t;
typedef union data_union *union_ptr_t;

#endif /* GTY_TEST_H */
