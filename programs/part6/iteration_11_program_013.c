/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_UNDEFINED: Type without GTY marker but referenced by annotated types */
struct undefined_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) base;  /* TYPE_POINTER */
    int scores[10];                         /* TYPE_ARRAY within struct */
    enum color color;                       /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char *GTY((tag("user_data"))) data;    /* TYPE_STRING pointer */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;          /* TYPE_STRING pointer */
    void *GTY((skip)) ptr_val;             /* TYPE_POINTER */
};

/* TYPE_ARRAY: Typedef for array type */
typedef int GTY(()) int_array_t[100];

/* Multi-dimensional array */
typedef struct basic_struct GTY(()) struct_matrix_t[10][10];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY((skip)) struct_ptr_t;
typedef int *GTY((skip)) int_ptr_t;
typedef void *GTY((skip)) void_ptr_t;

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((skip)) data;                /* Actual string data */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY((skip)) callback_func_t)(void *context, int param);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func_t GTY((skip)) handler;   /* TYPE_CALLBACK */
    void *GTY((skip)) user_data;
};

/* Linked list structure (for traversal) */
struct GTY(()) list_node {
    int value;
    struct list_node *GTY((skip)) next;    /* TYPE_POINTER to same type */
    struct list_node *GTY((skip)) prev;    /* TYPE_POINTER to same type */
};

/* Tree node structure */
struct GTY(()) tree_node {
    int key;
    struct tree_node *GTY((skip)) left;    /* TYPE_POINTER */
    struct tree_node *GTY((skip)) right;   /* TYPE_POINTER */
    union data_union GTY((skip)) data;     /* TYPE_UNION */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* This typically requires special handling in GCC frontends */
struct GTY(()) lang_struct {
    int lang_specific;
    /* In real GCC, this would have language-specific fields */
};

/* Container with all types */
struct GTY(()) type_container {
    /* Basic types */
    scalar_int_t count;                    /* TYPE_SCALAR */
    scalar_double_t total;                 /* TYPE_SCALAR */
    
    /* Struct types */
    struct basic_struct GTY((skip)) basic; /* TYPE_STRUCT */
    struct complex_struct *GTY((skip)) complex_ptr; /* TYPE_POINTER */
    
    /* Union */
    union data_union current_union;        /* TYPE_UNION */
    
    /* Arrays */
    int_array_t numbers;                   /* TYPE_ARRAY */
    struct_matrix_t matrix;                /* TYPE_ARRAY of TYPE_STRUCT */
    
    /* Pointers */
    struct_ptr_t struct_ptr;               /* TYPE_POINTER */
    int_ptr_t int_ptr;                     /* TYPE_POINTER */
    
    /* String */
    struct gcc_string GTY((skip)) text;    /* TYPE_STRING */
    
    /* Callback */
    callback_func_t callback;              /* TYPE_CALLBACK */
    
    /* Linked structures */
    struct list_node *GTY((skip)) head;    /* TYPE_POINTER */
    struct tree_node *GTY((skip)) root;    /* TYPE_POINTER */
    
    /* Undefined reference */
    struct undefined_struct *GTY((skip)) undefined_ptr; /* TYPE_POINTER to TYPE_UNDEFINED */
    
    /* Language struct */
    struct lang_struct GTY((skip)) lang;   /* TYPE_LANG_STRUCT */
    
    /* User struct */
    struct user_struct *GTY((skip)) user;  /* TYPE_POINTER to TYPE_USER_STRUCT */
};

/* Root structure for gengtype traversal */
extern struct type_container GTY(()) root_container;

#endif /* GTY_TEST_H */
