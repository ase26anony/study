/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Non-GTY annotated struct (will be referenced from GTY types) */
struct undefined_struct {
    int id;
    char name[32];
};

/* TYPE_STRUCT: Basic GTY-annotated struct */
struct GTY(()) basic_struct {
    int id;
    char *name;
    double value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *base;      /* TYPE_POINTER to TYPE_STRUCT */
    int count;
    struct undefined_struct *undef; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* TYPE_UNION: GTY-annotated union */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;               /* TYPE_POINTER to TYPE_SCALAR (char) */
    void *ptr_val;
};

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];
typedef struct basic_struct* struct_ptr_array_t[5];

struct GTY(()) array_container {
    int fixed_array[20];            /* Fixed-size array */
    struct_ptr_array_t ptr_array;   /* Array of pointers to struct */
    int_array_t ints;               /* Array of ints */
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *data;                     /* TYPE_POINTER to TYPE_SCALAR */
    const char *const_data;         /* Const pointer */
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *data, int param);
typedef void (*simple_callback_t)(void);

struct GTY(()) callback_container {
    callback_func_t handler;        /* TYPE_CALLBACK */
    simple_callback_t cleanup;
    void *user_data;
};

/* Linked list structure for chained references */
struct GTY(()) list_node {
    int data;
    struct list_node *next;         /* TYPE_POINTER to self */
    struct list_node *prev;
    union data_union node_data;     /* TYPE_UNION */
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef struct undefined_struct* undefined_ptr_t;
typedef int* int_ptr_t;
typedef callback_func_t* callback_ptr_t;

/* Mixed pointer container */
struct GTY(()) pointer_mix {
    basic_ptr_t struct_ptr;
    int_ptr_t int_ptr;
    void *void_ptr;
    struct gcc_string *string_ptr;  /* TYPE_POINTER to TYPE_STRING */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    struct GTY((skip)) skipped_part {
        int temp;
    } skip;
    struct pointer_mix *mix_ptr;
};

/* Root structure containing references to all types */
struct GTY(()) root_container {
    /* Basic types */
    struct basic_struct basic;
    struct user_struct user;
    
    /* Pointers */
    struct complex_struct *complex;
    struct array_container *arrays;
    struct gcc_string *string;
    
    /* Callbacks */
    struct callback_container callbacks;
    
    /* Linked structure */
    struct list_node *head;
    struct list_node *tail;
    
    /* Unions and misc */
    union data_union current_data;
    struct lang_struct lang;
    struct pointer_mix pointers;
    
    /* Arrays */
    struct basic_struct struct_array[3];
    callback_func_t callback_array[2];
    
    /* Scalar members */
    scalar_int_t scalar_int;
    scalar_double_t scalar_double;
    enum color color;
};

/* External declaration for gengtype to process */
extern struct root_container GTY((tag("ROOT"))) global_root;

#endif /* GTY_TEST_H */
