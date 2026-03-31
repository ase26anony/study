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

/* TYPE_UNDEFINED: Type without GTY marker (will be referenced from annotated types) */
struct undefined_struct {
    int id;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char *data;
    double value;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;
};

/* Nested struct for complex type graph */
struct GTY(()) inner_struct {
    int inner_value;
    char inner_char;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char *string_val;
    double double_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef int* int_ptr_t;
typedef void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];
typedef char char_array_t[256];
typedef struct basic_struct* struct_ptr_array_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *data;
    int capacity;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(void*, int, char*);
typedef struct basic_struct* (*struct_factory_t)(int id);

/* Struct containing callback */
struct GTY(()) callback_container {
    simple_callback_t cb1;
    complex_callback_t cb2;
    struct_factory_t factory;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    union {
        int int_val;
        double double_val;
        struct basic_struct* struct_ptr;
    } GTY((desc("%0.lang_specific_tag"))) lang_union;
};

/* Linked list structure for type traversal */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((skip)) next;  /* Skip for circular reference handling */
    struct list_node* GTY((skip)) prev;
};

/* Complex struct with multiple type combinations */
struct GTY(()) complex_type {
    /* Scalar members */
    int id;
    enum color color;
    double weight;
    
    /* Pointer members */
    struct basic_struct* basic_ptr;
    struct undefined_struct* undefined_ptr;  /* Will trigger TYPE_UNDEFINED */
    int_ptr_t int_ptr;
    
    /* Array members */
    int_array_t numbers;
    char_array_t buffer;
    struct_ptr_array_t struct_ptrs;
    
    /* Union member */
    union data_union data;
    
    /* String member */
    struct gcc_string description;
    
    /* Callback member */
    simple_callback_t callback;
    
    /* Nested struct */
    struct inner_struct inner;
    
    /* Language struct */
    struct lang_struct lang_data;
    
    /* Linked list */
    struct list_node* list_head;
};

/* Container with array of pointers to different types */
struct GTY(()) type_container {
    struct basic_struct* basics[5];
    union data_union unions[3];
    struct gcc_string strings[4];
    struct complex_type* complex;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_type {
    /* Direct pointers */
    struct basic_struct* basic;
    struct user_struct* user;
    struct complex_type* complex;
    struct type_container* container;
    struct callback_container* callbacks;
    
    /* Arrays of different types */
    struct basic_struct* basic_array[10];
    struct complex_type* complex_array[3];
    
    /* Union */
    union data_union root_union;
    
    /* String */
    struct gcc_string root_string;
    
    /* Linked list */
    struct list_node* linked_list;
    
    /* Self-reference for graph */
    struct root_type* GTY((skip)) self;
};

/* External declaration to force type inclusion */
extern struct root_type GTY((extern)) *global_root;

#endif /* GTY_TEST_H */
