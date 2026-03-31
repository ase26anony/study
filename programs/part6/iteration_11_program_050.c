/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

/* TYPE_UNDEFINED: Non-GTY annotated struct that will be referenced */
struct undefined_helper {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    int id;                     /* TYPE_SCALAR */
    char name[32];              /* TYPE_ARRAY */
    double values[10];          /* TYPE_ARRAY */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;            /* TYPE_STRING pointer */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;                /* TYPE_SCALAR */
    float float_val;            /* TYPE_SCALAR */
    double double_val;          /* TYPE_SCALAR */
    char* string_val;           /* TYPE_STRING pointer */
    void* ptr_val;              /* TYPE_POINTER */
};

/* TYPE_POINTER: Various pointer types */
typedef basic_struct* struct_ptr_t;
typedef int* int_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* TYPE_ARRAY: Array typedefs */
typedef int int_array_10_t[10];
typedef struct_ptr_t ptr_array_5_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;                 /* TYPE_SCALAR */
    char* data;                 /* TYPE_STRING pointer */
    const char* const_data;     /* TYPE_STRING pointer */
};

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*callback_func_t)(void* context, int value);
typedef void (*simple_callback_t)(void);

/* Nested structure with callback */
struct GTY(()) callback_container {
    callback_func_t handler;    /* TYPE_CALLBACK */
    void* user_data;            /* TYPE_POINTER */
    int state;                  /* TYPE_SCALAR */
};

/* Linked list structure for traversal */
struct GTY(()) list_node {
    int data;                   /* TYPE_SCALAR */
    struct list_node* next;     /* TYPE_POINTER to self */
    struct list_node* prev;     /* TYPE_POINTER to self */
    void* payload;              /* TYPE_POINTER */
};

/* Complex structure mixing all types */
struct GTY(()) complex_type {
    /* Scalars */
    int id;                     /* TYPE_SCALAR */
    enum { RED, GREEN, BLUE } color; /* TYPE_SCALAR (enum) */
    
    /* Pointers */
    struct basic_struct* base;  /* TYPE_POINTER */
    int* int_ptr;               /* TYPE_POINTER */
    
    /* Arrays */
    int scores[5];              /* TYPE_ARRAY */
    struct_ptr_t struct_ptrs[3]; /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Union */
    union data_union storage;   /* TYPE_UNION */
    
    /* String */
    struct gcc_string title;    /* TYPE_STRUCT (string) */
    
    /* Callback */
    callback_func_t notify;     /* TYPE_CALLBACK */
    
    /* Reference to undefined type */
    struct undefined_helper* helper; /* TYPE_POINTER to TYPE_UNDEFINED */
    
    /* Nested structure */
    struct {
        int x, y;               /* TYPE_SCALAR */
    } position;                 /* Anonymous TYPE_STRUCT */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
    int lang_specific;
    void* lang_data;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Direct instances */
    struct basic_struct basic;          /* TYPE_STRUCT */
    struct user_struct user;            /* TYPE_USER_STRUCT */
    union data_union union_data;        /* TYPE_UNION */
    struct gcc_string string_data;      /* TYPE_STRUCT (string) */
    struct callback_container callback; /* TYPE_STRUCT with callback */
    struct list_node* list_head;        /* TYPE_POINTER */
    struct complex_type complex;        /* TYPE_STRUCT */
    struct lang_struct lang;            /* TYPE_LANG_STRUCT */
    
    /* Arrays of different types */
    struct basic_struct struct_array[4];    /* TYPE_ARRAY of TYPE_STRUCT */
    union data_union union_array[2];        /* TYPE_ARRAY of TYPE_UNION */
    callback_func_t callback_array[3];      /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* Pointer arrays */
    void* void_ptr_array[8];                /* TYPE_ARRAY of TYPE_POINTER */
    struct gcc_string* string_ptr_array[4]; /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Multi-dimensional array */
    int matrix[3][3];                       /* TYPE_ARRAY (multi-dim) */
};

/* External variable to force inclusion in GC roots */
extern struct root_container GTY((root)) global_root;

#endif /* GTY_TEST_H */
