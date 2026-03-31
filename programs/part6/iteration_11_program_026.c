/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (processed as scalar) */
enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Type without GTY marker, referenced by annotated types */
struct undefined_struct {
    int id;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    int id;                    /* TYPE_SCALAR */
    char *name;               /* TYPE_STRING */
    struct undefined_struct *ref;  /* TYPE_UNDEFINED reference */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;          /* Opaque pointer */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;              /* TYPE_SCALAR */
    double double_val;        /* TYPE_SCALAR */
    char *string_val;         /* TYPE_STRING */
    struct base_struct *struct_ptr; /* TYPE_POINTER */
};

/* TYPE_ARRAY: Array typedef */
typedef int GTY(()) int_array_t[10];

/* Fixed-size array within struct */
struct GTY(()) array_container {
    int_array_t fixed_array;  /* TYPE_ARRAY */
    char buffer[256];         /* TYPE_ARRAY */
    struct base_struct *ptr_array[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct * GTY(()) base_ptr_t;
typedef int * GTY(()) int_ptr_t;
typedef void * GTY(()) void_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef int (* GTY(()) callback_func_t)(void *data, int param);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func_t handler;  /* TYPE_CALLBACK */
    void *user_data;          /* TYPE_POINTER */
    int (*inline_callback)(struct base_struct *); /* TYPE_CALLBACK */
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;               /* TYPE_SCALAR */
    char * GTY((length("length"))) data; /* TYPE_STRING with length */
};

/* Linked list for traversal */
struct GTY(()) list_node {
    int value;                /* TYPE_SCALAR */
    struct list_node * GTY((skip)) next; /* TYPE_POINTER with skip */
    struct list_node *prev;   /* TYPE_POINTER */
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct base_struct base;          /* TYPE_STRUCT */
    union data_union variant;         /* TYPE_UNION */
    struct array_container arrays;    /* TYPE_STRUCT */
    struct gcc_string str;            /* TYPE_STRUCT (string) */
    callback_func_t callbacks[3];     /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_tag"), tag("LANG_STRUCT"))) lang_struct {
    int lang_specific;
    void *lang_data;
};

/* Root structure containing pointers to all types */
struct GTY(()) type_root {
    /* Basic types */
    struct base_struct *base_ptr;      /* TYPE_POINTER */
    struct user_struct *user_ptr;      /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* Containers */
    union data_union *union_ptr;       /* TYPE_POINTER to TYPE_UNION */
    struct array_container *array_ptr; /* TYPE_POINTER */
    struct callback_container *cb_ptr; /* TYPE_POINTER */
    
    /* Strings and lists */
    struct gcc_string *string_ptr;     /* TYPE_POINTER to TYPE_STRUCT (string) */
    struct list_node *list_head;       /* TYPE_POINTER */
    
    /* Nested and language */
    struct complex_nested *nested_ptr; /* TYPE_POINTER */
    struct lang_struct *lang_ptr;      /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Arrays of various types */
    struct base_struct *struct_array[8]; /* TYPE_ARRAY of TYPE_POINTER */
    callback_func_t callback_array[4];   /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* Direct members */
    int scalar_field;                  /* TYPE_SCALAR */
    char *string_field;                /* TYPE_STRING */
    int_ptr_t int_pointer;             /* TYPE_POINTER */
    
    /* Union directly embedded */
    union data_union direct_union;     /* TYPE_UNION */
    
    /* Array directly embedded */
    int direct_array[20];              /* TYPE_ARRAY */
};

/* Additional pointer chain for deep traversal */
struct GTY(()) pointer_chain {
    struct pointer_chain *next;        /* TYPE_POINTER */
    struct pointer_chain *prev;        /* TYPE_POINTER */
    void *data;                        /* TYPE_POINTER */
    int depth;                         /* TYPE_SCALAR */
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;                         /* TYPE_SCALAR */
    struct tree_node *left;            /* TYPE_POINTER */
    struct tree_node *right;           /* TYPE_POINTER */
    struct tree_node *parent;          /* TYPE_POINTER */
};

#endif /* GTY_TEST_H */
