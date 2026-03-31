/* Test header for gengtype coverage testing */
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

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *user_data);

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unmarked_struct {
    int x;
    double y;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined {
    int id;
    char *name;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    int id;
    double value;
    char tag;
};

/* Another struct with pointer members */
struct GTY(()) struct_with_pointers {
    int count;
    struct basic_struct * GTY((tag("0"))) items;
    struct gcc_string * GTY((skip)) description;
};

/* TYPE_ARRAY: Array types */
typedef int vec4_t[4];
typedef struct basic_struct *struct_ptr_array_t[10];

/* Struct containing arrays */
struct GTY(()) struct_with_arrays {
    int matrix[3][3];
    vec4_t vector;
    struct_ptr_array_t pointers;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int as_int;
    double as_double;
    char * GTY((skip)) as_string;
    struct basic_struct * GTY((tag("1"))) as_struct;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef union data_union *union_ptr_t;
typedef callback_t *callback_ptr_t;

/* Linked list structure (for traversal) */
struct GTY(()) linked_node {
    int data;
    struct linked_node * GTY((tag("2"))) next;
    struct linked_node * GTY((tag("3"))) prev;
};

/* Tree node structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node * GTY((tag("4"))) left;
    struct tree_node * GTY((tag("5"))) right;
    union data_union payload;
};

/* Struct with callback member */
struct GTY(()) struct_with_callback {
    int id;
    callback_t handler;
    void * GTY((skip)) user_data;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct struct_with_arrays arrays;
    union data_union choice;
    struct linked_node * GTY((tag("6"))) list_head;
    struct tree_node * GTY((tag("7"))) tree_root;
    callback_t validators[3];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_flag;
    void * GTY((skip)) lang_data;
    struct GTY((tag("8"))) lang_struct *next;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Basic types */
    scalar_int_t counter;
    scalar_double_t total;
    
    /* String type */
    struct gcc_string title;
    
    /* Various struct types */
    struct basic_struct basic;
    struct struct_with_pointers * GTY((tag("9"))) ptr_struct;
    struct struct_with_arrays array_struct;
    
    /* Union */
    union data_union data;
    
    /* Pointers */
    basic_ptr_t basic_ptr;
    union_ptr_t union_ptr;
    callback_ptr_t callback_ptr;
    
    /* Linked structures */
    struct linked_node * GTY((tag("10"))) list;
    struct tree_node * GTY((tag("11"))) tree;
    
    /* Callback */
    struct struct_with_callback callback_struct;
    
    /* Complex nested */
    struct complex_nested * GTY((tag("12"))) nested;
    
    /* Language struct */
    struct lang_struct * GTY((tag("13"))) lang_chain;
    
    /* User struct */
    struct user_defined * GTY((tag("14"))) user_data;
    
    /* Array of pointers */
    struct basic_struct * GTY((tag("15"))) ptr_array[5];
    
    /* Mixed array */
    union data_union mixed_array[8];
};

/* External declaration to force inclusion in type graph */
extern struct root_container * GTY((tag("16"))) global_root;

#endif /* GTY_TEST_H */
