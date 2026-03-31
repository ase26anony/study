/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

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
    char *name;                 /* TYPE_POINTER to TYPE_STRING */
    double value;               /* TYPE_SCALAR */
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *base;  /* TYPE_POINTER to TYPE_STRUCT */
    int counts[10];             /* TYPE_ARRAY of TYPE_SCALAR */
    enum color color;           /* TYPE_SCALAR (enum) */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *user_data;            /* TYPE_POINTER */
    int user_id;                /* TYPE_SCALAR */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;                /* TYPE_SCALAR */
    double double_val;          /* TYPE_SCALAR */
    char *string_val;           /* TYPE_POINTER to TYPE_STRING */
    void *ptr_val;              /* TYPE_POINTER */
};

/* TYPE_ARRAY: Typedef for array type */
typedef int GTY(()) int_array_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;                 /* TYPE_SCALAR */
    char *data;                 /* TYPE_POINTER to TYPE_STRING */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int GTY(()) (*callback_func)(void *context, int value);

/* TYPE_CALLBACK in struct */
struct GTY(()) callback_container {
    callback_func handler;      /* TYPE_CALLBACK */
    void *context;              /* TYPE_POINTER */
    int state;                  /* TYPE_SCALAR */
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr_t;
typedef int* GTY(()) int_ptr_t;
typedef void (*GTY(()) raw_func_ptr)(void);

/* Linked list for traversal */
struct GTY(()) list_node {
    int data;                   /* TYPE_SCALAR */
    struct list_node *next;     /* TYPE_POINTER to TYPE_STRUCT */
    struct list_node *prev;     /* TYPE_POINTER to TYPE_STRUCT */
};

/* Array of pointers */
struct GTY(()) pointer_array {
    void* GTY((skip)) ptrs[8];  /* TYPE_ARRAY of TYPE_POINTER */
    int count;                  /* TYPE_SCALAR */
};

/* Mixed type container */
struct GTY(()) mixed_container {
    union data_union u_data;    /* TYPE_UNION */
    struct gcc_string str;      /* TYPE_STRUCT (TYPE_STRING) */
    int_array_t numbers;        /* TYPE_ARRAY */
    callback_func cb;           /* TYPE_CALLBACK */
    struct undefined_struct* undef_ref; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    struct lang_struct *next;   /* TYPE_POINTER */
    struct lang_struct *prev;   /* TYPE_POINTER */
    int lang_specific;          /* TYPE_SCALAR */
};

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    struct basic_struct *basic;         /* TYPE_POINTER to TYPE_STRUCT */
    struct complex_struct *complex;     /* TYPE_POINTER to TYPE_STRUCT */
    struct user_struct *user;           /* TYPE_POINTER to TYPE_USER_STRUCT */
    union data_union *union_ptr;        /* TYPE_POINTER to TYPE_UNION */
    struct gcc_string *string_obj;      /* TYPE_POINTER to TYPE_STRUCT (TYPE_STRING) */
    struct callback_container *cb_cont; /* TYPE_POINTER to TYPE_STRUCT */
    struct list_node *list_head;        /* TYPE_POINTER to TYPE_STRUCT */
    struct pointer_array *ptr_array;    /* TYPE_POINTER to TYPE_STRUCT */
    struct mixed_container *mixed;      /* TYPE_POINTER to TYPE_STRUCT */
    struct lang_struct *lang;           /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Direct members */
    int scalar_field;                   /* TYPE_SCALAR */
    char *string_field;                 /* TYPE_POINTER to TYPE_STRING */
    int_array_t array_field;            /* TYPE_ARRAY */
    callback_func direct_callback;      /* TYPE_CALLBACK */
};

/* Global root variable */
extern struct root_container GTY(()) global_root;

#endif /* GTY_TEST_H */
