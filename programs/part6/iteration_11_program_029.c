/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (processed as scalar) */
enum color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *user_data, int value);

/* Non-annotated struct (may become TYPE_UNDEFINED when referenced) */
struct unannotated_struct {
    int x;
    double y;
    char *name;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_defined {
    int id;
    char *GTY((tag("name"))) username;
    void *GTY((skip)) private_data;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;
    struct gcc_string *str_obj;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct gcc_string *string_array_t[10];

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    int id;
    scalar_int_t count;
    scalar_double_t value;
    enum color color;
};

/* TYPE_STRUCT with nested struct */
struct GTY(()) nested_struct {
    struct basic_struct inner;
    struct nested_struct *GTY((skip)) self_ref;
};

/* TYPE_STRUCT with pointer members */
struct GTY(()) pointer_struct {
    int *int_ptr;
    struct basic_struct *struct_ptr;
    struct unannotated_struct *unannotated_ptr;  /* May trigger TYPE_UNDEFINED */
    union data_union *union_ptr;
};

/* TYPE_STRUCT with array members */
struct GTY(()) array_struct {
    int fixed_array[5];
    vec4_t vector;
    string_array_t strings;
    struct basic_struct *ptr_array[8];
};

/* TYPE_STRUCT with callback */
struct GTY(()) callback_struct {
    callback_t handler;
    void *user_data;
    int (*direct_func)(int, int);
};

/* Linked list structure (chain of types) */
struct GTY(()) list_node {
    int data;
    struct list_node *next;
    struct list_node *prev;
};

/* Complex structure mixing everything */
struct GTY(()) complex_type {
    /* Scalar fields */
    int id;
    double weight;
    enum color primary_color;
    
    /* Pointer fields */
    struct basic_struct *base;
    struct pointer_struct *ptr_struct;
    struct array_struct *array_struct;
    struct callback_struct *callback_struct;
    
    /* Union field */
    union data_union variant;
    
    /* Array fields */
    int matrix[3][3];
    struct list_node *node_array[5];
    
    /* String field */
    struct gcc_string description;
    
    /* Callback field */
    callback_t notify;
    
    /* Reference to user struct */
    struct user_defined *user;
    
    /* Reference to potentially undefined type */
    struct unannotated_struct *maybe_undefined;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    void *GTY((desc("%1.lang_specific_tag"))) lang_data;
};

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    struct basic_struct *basic;
    struct pointer_struct *pointer;
    struct array_struct *array;
    struct callback_struct *callback;
    struct nested_struct *nested;
    struct list_node *list_head;
    struct complex_type *complex;
    struct lang_struct *lang_specific;
    union data_union *union_obj;
    struct gcc_string *string_obj;
    struct user_defined *user_struct;
    
    /* Array of various pointers */
    void *GTY((skip)) void_ptr_array[20];
    
    /* Multi-dimensional array */
    struct basic_struct *struct_matrix[4][4];
};

/* External variable declarations for roots */
extern struct root_container GTY((root)) global_root;
extern struct list_node GTY((chain)) *global_list;

#endif /* GTY_TEST_H */
