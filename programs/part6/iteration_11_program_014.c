/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;

enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Non-GTY annotated struct referenced by GTY types */
struct undefined_helper {
    int x;
    float y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;
    struct undefined_helper* helper;  /* References undefined type */
};

/* Nested struct for complex relationships */
struct GTY(()) inner_struct {
    int counter;
    float ratio;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef int* int_ptr_t;
typedef void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct* ptr_array_5[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int GTY((skip)) length;  /* skip marker for coverage */
    char* GTY((length("length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void* context, int param);
typedef void (*simple_callback_t)(void);

/* More complex struct with multiple type references */
struct GTY(()) complex_struct {
    /* TYPE_SCALAR members */
    int id;
    enum color color;
    
    /* TYPE_POINTER members */
    struct basic_struct* basic_ptr;
    struct user_struct* user_ptr;
    
    /* TYPE_ARRAY members */
    int scores[5];
    struct inner_struct* object_array[3];
    
    /* TYPE_UNION member */
    union data_union storage;
    
    /* TYPE_STRING member */
    struct gcc_string description;
    
    /* TYPE_CALLBACK member */
    callback_func_t handler;
    
    /* Chain pointer for linked list */
    struct complex_struct* GTY((skip)) next;
};

/* Linked list structure for traversal */
struct GTY(()) node {
    int data;
    struct node* next;
    struct node* prev;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
    void* lang_data;
};
#endif

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    /* Basic types */
    struct basic_struct* basic;
    struct user_struct* user;
    struct inner_struct* inner;
    
    /* Collections */
    struct complex_struct* complex;
    struct node* list_head;
    
    /* Special types */
    struct gcc_string* title;
    union data_union* variant;
    
    /* Arrays */
    int_array_10 numbers;
    ptr_array_5 pointers;
    
    /* Callback */
    simple_callback_t init_func;
    
    /* Reference to undefined type */
    struct undefined_helper* helper_ref;
};

/* External declaration for gengtype to process */
extern struct root_container GTY((tag("ROOT"))) global_root;

#endif /* GTY_TEST_H */
