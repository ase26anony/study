/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_UNDEFINED: Non-GTY type that will be referenced */
struct non_gty_struct {
    int x;
    char y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) ptr_to_basic;
    int count;
    struct complex_struct *GTY((skip)) next;  /* Linked list */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *GTY((skip)) user_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;
    void *GTY((skip)) ptr_val;
};

/* TYPE_ARRAY: Array types */
typedef int GTY((length("len"))) int_array_t[];
typedef struct basic_struct GTY((skip)) struct_array_t[10];

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr_t;
typedef struct basic_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int GTY((skip)) length;
    char *GTY((skip)) data;
    const char *GTY((skip)) const_data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *GTY((skip)) context, int param);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func_t GTY((skip)) handler;
    void *GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%1.type"), tag("lang_type"))) lang_struct {
    enum { LANG_INT, LANG_FLOAT, LANG_PTR } type;
    union {
        int int_val;
        double float_val;
        void *GTY((skip)) ptr_val;
    } GTY((desc ("%0.type"))) value;
};

/* Linked list for deep traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
};

/* Tree structure for complex traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of pointers */
struct GTY(()) pointer_array_container {
    void *GTY((skip)) pointers[8];
    int *GTY((skip)) int_ptrs[4];
    struct basic_struct *GTY((skip)) struct_ptrs[5];
};

/* Mixed type container */
struct GTY(()) mixed_container {
    /* TYPE_SCALAR members */
    int scalar_int;
    double scalar_double;
    char scalar_char;
    
    /* TYPE_POINTER members */
    int *int_ptr;
    struct basic_struct *struct_ptr;
    void *void_ptr;
    
    /* TYPE_ARRAY members */
    int fixed_array[16];
    char string_buffer[256];
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_STRING member */
    struct gcc_string str;
    
    /* TYPE_CALLBACK member */
    callback_func_t callback;
    
    /* Reference to non-GTY type (TYPE_UNDEFINED) */
    struct non_gty_struct *non_gty_ref;
    
    /* Reference to user struct */
    struct user_struct *user_struct_ref;
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct basic_struct basic;
    struct complex_struct complex;
    struct user_struct user;
    union data_union union_data;
    struct gcc_string string_data;
    struct lang_struct lang_data;
    struct list_node *list_head;
    struct tree_node *tree_root;
    struct pointer_array_container ptr_array;
    struct mixed_container mixed;
    struct callback_container callback_wrapper;
    
    /* Array of various types */
    struct basic_struct struct_array[5];
    union data_union union_array[3];
    callback_func_t callback_array[4];
};

/* External declaration to force inclusion */
extern struct root_container GTY((skip)) *global_root;

#endif /* GTY_TEST_H */
