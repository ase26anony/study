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

/* TYPE_UNDEFINED: Non-GTY annotated struct referenced by GTY types */
struct undefined_helper {
    int x;
    float y;
};

/* TYPE_STRUCT: Basic GTY struct */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) basic_ptr;
    int count;
    struct undefined_helper *helper;  /* References undefined type */
    enum color color;
};

/* TYPE_USER_STRUCT: User-defined structure */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((tag("STRING"))) string_val;
    struct basic_struct *struct_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;
typedef int *int_ptr_t;
typedef void *void_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];
typedef struct basic_struct *struct_ptr_array_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((length("length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *data, int param);
typedef void (*simple_callback_t)(void);

/* TYPE_STRUCT with callback member */
struct GTY(()) callback_container {
    callback_func_t callback;
    void *callback_data;
    simple_callback_t cleanup;
};

/* Linked list structure for traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;
    struct list_node *prev;
};

/* Array container */
struct GTY(()) array_container {
    int_array_t fixed_array;
    struct_ptr_array_t ptr_array;
    int dynamic_array[0];  /* Zero-length array */
};

/* Union container */
struct GTY(()) union_container {
    int type;
    union {
        int int_member;
        double double_member;
        struct gcc_string *string_member;
    } GTY((desc("type"))) value;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    int lang_specific;
    struct lang_struct *next;
    struct lang_struct *prev;
    void *lang_data;
};

/* Root structure containing all types */
struct GTY(()) root_container {
    /* Basic types */
    scalar_int_t scalar_int;
    scalar_double_t scalar_double;
    
    /* Struct pointers */
    struct basic_struct *basic;
    struct complex_struct *complex;
    struct user_struct *user;
    
    /* Union */
    union data_union data;
    
    /* Arrays */
    int_array_t numbers;
    struct_ptr_array_t structs;
    
    /* String */
    struct gcc_string *string;
    
    /* Callbacks */
    callback_func_t func_ptr;
    struct callback_container *callback;
    
    /* Linked list */
    struct list_node *list_head;
    struct list_node *list_tail;
    
    /* Array container */
    struct array_container *arrays;
    
    /* Union container */
    struct union_container *unions;
    
    /* Language structure */
    struct lang_struct *lang;
    
    /* Various pointers */
    int_ptr_t int_ptr;
    void_ptr_t void_ptr;
    
    /* Nested structure */
    struct {
        int nested_id;
        char nested_name[64];
    } GTY((skip)) nested;
};

/* External declaration to force type registration */
extern struct root_container * GTY((root)) global_root;

#endif /* GTY_TEST_H */
