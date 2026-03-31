/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (processed as scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *data, int value);

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct GTY((user)) user_defined {
    int id;
    char *name;
};

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct plain_struct {
    int x, y;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) base_struct {
    int id;
    char * GTY((skip)) name;
    enum color color;
};

/* TYPE_UNION: Annotated union */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char * GTY((skip)) string_val;
    void *ptr_val;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct base_struct *struct_ptr_array_t[10];

/* TYPE_POINTER: Various pointer types */
typedef base_struct *base_ptr_t;
typedef void *generic_ptr_t;

/* Complex nested structure with multiple type references */
struct GTY(()) complex_node {
    int data;
    
    /* TYPE_POINTER to same type (linked list) */
    struct complex_node * GTY((tag("0"))) next;
    
    /* TYPE_POINTER to different struct */
    struct base_struct *base_ptr;
    
    /* TYPE_UNION member */
    union data_union value;
    
    /* TYPE_ARRAY member */
    int scores[5];
    
    /* TYPE_CALLBACK member */
    callback_t callback;
    
    /* TYPE_STRING member */
    struct gcc_string *description;
    
    /* TYPE_ARRAY of pointers */
    void * GTY((skip)) ptr_array[8];
    
    /* Nested anonymous struct */
    struct {
        int x, y;
    } position;
};

/* Another struct with array of structs */
struct GTY(()) container {
    int count;
    
    /* TYPE_ARRAY of structs */
    struct complex_node nodes[4];
    
    /* TYPE_ARRAY of pointers to union */
    union data_union *union_ptrs[3];
    
    /* TYPE_POINTER to callback */
    callback_t *callback_ptr;
    
    /* Reference to non-annotated type */
    struct plain_struct *plain;
    
    /* User struct reference */
    struct user_defined *user_data;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY((desc("%1"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int lang_specific;
    struct lang_struct *next;
    struct lang_struct *prev;
};
#endif

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* TYPE_POINTER to various structs */
    struct base_struct *base;
    struct complex_node *node_list;
    struct container *main_container;
    
    /* TYPE_UNION */
    union data_union current_data;
    
    /* TYPE_ARRAY of strings */
    struct gcc_string *strings[5];
    
    /* TYPE_CALLBACK array */
    callback_t callbacks[3];
    
    /* TYPE_SCALAR members */
    scalar_int_t counter;
    scalar_double_t total;
    enum color default_color;
    
    /* TYPE_ARRAY of scalars */
    int values[20];
    
    /* TYPE_POINTER to array type */
    vec4_t *vector_ptr;
    
    /* Reference to user struct */
    struct user_defined *user_info;
    
    #ifdef GENERATOR_FILE
    struct lang_struct *lang_data;
    #endif
};

/* External declaration to force type inclusion */
extern struct root_container GTY((extern)) *global_root;

#endif /* GTY_TEST_H */
