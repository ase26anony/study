/* Test header for gengtype-state.cc coverage testing */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

/* TYPE_ENUM (handled as scalar) */
enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Type without GTY marker, referenced by annotated types */
struct undefined_struct {
    int x;
    double y;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;                     /* TYPE_SCALAR */
    char *name;                 /* TYPE_STRING */
    double values[3];           /* TYPE_ARRAY of scalars */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;            /* TYPE_POINTER */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;                /* TYPE_SCALAR */
    double double_val;          /* TYPE_SCALAR */
    char *string_val;           /* TYPE_STRING */
    void *ptr_val;              /* TYPE_POINTER */
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef int* int_ptr_t;
typedef void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct* ptr_array_5[5];
typedef double matrix_3x3[3][3];

/* TYPE_STRING: String representation */
struct GTY(()) gcc_string {
    int length;
    char *data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *context, int value);
typedef void (*simple_callback_t)(void);

/* Complex nested structure with multiple type references */
struct GTY(()) complex_node {
    int node_id;                            /* TYPE_SCALAR */
    struct complex_node *next;              /* TYPE_POINTER to struct */
    struct complex_node *prev;              /* TYPE_POINTER to struct */
    union data_union data;                  /* TYPE_UNION */
    callback_func_t callback;               /* TYPE_CALLBACK */
    struct undefined_struct *undef_ref;     /* TYPE_POINTER to undefined */
    struct user_struct *user_ref;           /* TYPE_POINTER to user struct */
    int_array_10 numbers;                   /* TYPE_ARRAY */
    struct gcc_string description;          /* TYPE_STRING */
};

/* Linked list structure */
struct GTY(()) linked_list {
    struct complex_node *head;              /* TYPE_POINTER */
    struct complex_node *tail;              /* TYPE_POINTER */
    int count;                              /* TYPE_SCALAR */
};

/* Container with arrays of different types */
struct GTY(()) type_container {
    /* Array of structs */
    struct basic_struct structs[5];         /* TYPE_ARRAY of structs */
    
    /* Array of pointers */
    struct complex_node *nodes[20];         /* TYPE_ARRAY of pointers */
    
    /* Array of unions */
    union data_union unions[8];             /* TYPE_ARRAY of unions */
    
    /* Array of strings */
    struct gcc_string strings[4];           /* TYPE_ARRAY of strings */
    
    /* Array of callbacks */
    callback_func_t callbacks[3];           /* TYPE_ARRAY of callbacks */
    
    /* Multi-dimensional array */
    matrix_3x3 transformation;              /* TYPE_ARRAY (2D) */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%1.lang_tag"), chain_next("%h.next"))) lang_struct {
    int lang_tag;
    struct lang_struct *next;
    void *language_data;
};

/* Root structure containing references to all types */
struct GTY(()) root_type {
    /* Direct type instances */
    struct basic_struct basic;              /* TYPE_STRUCT */
    struct user_struct user;                /* TYPE_USER_STRUCT */
    union data_union data;                  /* TYPE_UNION */
    struct gcc_string title;                /* TYPE_STRING */
    
    /* Pointers */
    struct complex_node *current;           /* TYPE_POINTER */
    struct linked_list *list;               /* TYPE_POINTER */
    struct type_container *container;       /* TYPE_POINTER */
    struct lang_struct *lang_chain;         /* TYPE_POINTER */
    
    /* Arrays */
    int_ptr_t int_ptrs[10];                 /* TYPE_ARRAY of pointers */
    basic_ptr_t struct_ptrs[5];             /* TYPE_ARRAY of pointers */
    
    /* Callback */
    callback_func_t handler;                /* TYPE_CALLBACK */
    
    /* Reference to undefined type */
    struct undefined_struct *undef;         /* TYPE_POINTER to undefined */
    
    /* Scalar members */
    scalar_int_t counter;                   /* TYPE_SCALAR */
    scalar_double_t weight;                 /* TYPE_SCALAR */
    enum color color;                       /* TYPE_SCALAR (enum) */
};

/* Global root variable */
extern struct root_type GTY(()) global_root;

#endif /* GTY_TEST_H */
