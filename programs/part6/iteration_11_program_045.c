/* Test header for gengtype-state.cc coverage testing */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_UNDEFINED: Type without GTY marker (will be referenced from GTY types) */
struct undefined_struct {
    int x;
    char y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    scalar_int_t id;
    scalar_double_t value;
    char name[32];  /* TYPE_ARRAY: Fixed-size array */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char *user_name;  /* TYPE_STRING: Pointer to string */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;  /* TYPE_STRING */
    void *ptr_val;     /* TYPE_POINTER */
};

/* TYPE_POINTER: Various pointer types */
typedef basic_struct *struct_ptr_t;
typedef int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int int_array_10_t[10];
typedef struct_ptr_t struct_ptr_array_t[5];

/* TYPE_STRING: String structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *context, int value);
typedef void (*simple_callback_t)(void);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};

/* Complex nested structure to trigger deep traversal */
struct GTY(()) complex_node {
    int data;
    
    /* TYPE_POINTER to same type (linked list) */
    struct complex_node *next;
    
    /* TYPE_POINTER to different struct */
    basic_struct *basic_ref;
    
    /* TYPE_UNION */
    data_union variant;
    
    /* TYPE_ARRAY */
    int scores[5];
    
    /* TYPE_CALLBACK as member */
    callback_func_t callback;
    
    /* TYPE_STRING */
    gcc_string *description;
    
    /* Reference to TYPE_UNDEFINED */
    struct undefined_struct *undef_ref;
};

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    /* TYPE_STRUCT references */
    basic_struct *basic;
    user_struct *user;
    
    /* TYPE_UNION */
    data_union main_union;
    
    /* TYPE_POINTER arrays */
    struct_ptr_array_t struct_ptrs;
    
    /* TYPE_ARRAY of scalars */
    int_array_10_t numbers;
    
    /* TYPE_STRING */
    gcc_string *title;
    
    /* TYPE_CALLBACK */
    callback_func_t handlers[3];  /* TYPE_ARRAY of callbacks */
    
    /* TYPE_LANG_STRUCT */
    lang_struct *lang;
    
    /* Linked list of complex nodes */
    complex_node *node_list;
    
    /* Mixed pointer types */
    int_ptr_t int_ptr;
    generic_ptr_t generic_ptr;
    
    /* Direct scalar members */
    scalar_int_t count;
    scalar_double_t total;
    
    /* Enum type (also TYPE_SCALAR) */
    enum GTY(()) color_enum {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE
    } color;
};

/* External root variable for gengtype to find */
extern GTY(()) root_container *global_root;

#endif /* GTY_TEST_H */
