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

/* TYPE_UNDEFINED: Type without GTY marker but referenced from annotated types */
struct undefined_struct {
    int id;
    char name[32];
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    int id;                     /* TYPE_SCALAR */
    char* name;                 /* TYPE_POINTER to TYPE_STRING */
    struct undefined_struct* ref; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* opaque_data;
    int user_tag;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;                /* TYPE_SCALAR */
    double double_val;          /* TYPE_SCALAR */
    char* string_val;           /* TYPE_POINTER to TYPE_STRING */
    struct base_struct* struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
};

/* TYPE_ARRAY: Array typedef */
typedef int GTY(()) int_array_t[10];

/* Fixed-size array within struct */
struct GTY(()) array_container {
    int_array_t fixed_array;    /* TYPE_ARRAY */
    int* dynamic_array GTY((length("dynamic_len"))); /* TYPE_POINTER with length */
    int dynamic_len;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* GTY(()) base_ptr_t;
typedef int* GTY(()) int_ptr_t;
typedef void (*void_func_ptr)(void); /* Function pointer */

/* TYPE_CALLBACK: Function pointer typedef */
typedef int GTY(()) (*callback_func_t)(void* context, int value);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func_t handler;    /* TYPE_CALLBACK */
    void* user_data;            /* TYPE_POINTER */
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;                 /* TYPE_SCALAR */
    char* data;                 /* TYPE_POINTER to char (string) */
};

/* Linked list for chained references */
struct GTY(()) list_node {
    int value;                  /* TYPE_SCALAR */
    struct list_node* next;     /* TYPE_POINTER to TYPE_STRUCT */
    struct list_node* prev;     /* TYPE_POINTER to TYPE_STRUCT */
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct base_struct base;    /* TYPE_STRUCT */
    union data_union variant;   /* TYPE_UNION */
    struct array_container arrays; /* TYPE_STRUCT with TYPE_ARRAY */
    struct gcc_string str;      /* TYPE_STRUCT (string-like) */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    void* lang_data GTY((skip)); /* Skip for GC */
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Various pointer types */
    struct base_struct* base_ptr;       /* TYPE_POINTER to TYPE_STRUCT */
    union data_union* union_ptr;        /* TYPE_POINTER to TYPE_UNION */
    struct array_container* array_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
    struct gcc_string* string_ptr;      /* TYPE_POINTER to TYPE_STRUCT */
    struct callback_container* callback_ptr; /* TYPE_POINTER to TYPE_STRUCT */
    struct user_struct* user_ptr;       /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct lang_struct* lang_ptr;       /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Direct members */
    int scalar_member;                  /* TYPE_SCALAR */
    double double_member;               /* TYPE_SCALAR */
    enum color color_member;            /* TYPE_SCALAR (enum) */
    
    /* Arrays */
    struct base_struct* ptr_array[5];   /* TYPE_ARRAY of TYPE_POINTER */
    int int_matrix[3][3];               /* TYPE_ARRAY multi-dimensional */
    
    /* Union */
    union data_union data;              /* TYPE_UNION */
    
    /* String */
    struct gcc_string title;            /* TYPE_STRUCT (string-like) */
    
    /* Callback */
    callback_func_t callback;           /* TYPE_CALLBACK */
    
    /* Linked list head */
    struct list_node* list_head;        /* TYPE_POINTER to TYPE_STRUCT */
    
    /* Reference to undefined type */
    struct undefined_struct* undefined_ref; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Global root variable */
extern struct root_container GTY(()) global_root;

#endif /* GTY_TEST_H */
