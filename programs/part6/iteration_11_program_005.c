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

/* TYPE_UNDEFINED: Forward declaration without GTY marker */
struct undefined_struct;

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) basic_ptr;
    int *GTY((skip)) int_ptr;
    enum color color;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char *GTY((skip)) user_name;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;
    void *GTY((skip)) ptr_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10_t[10];
typedef struct basic_struct struct_array_5_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int GTY((skip)) length;
    char *GTY((skip)) data;
    const char *GTY((skip)) const_data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*cleanup_func_t)(void *);

/* TYPE_CALLBACK in struct */
struct GTY(()) callback_container {
    comparator_t GTY((skip)) compare;
    cleanup_func_t GTY((skip)) cleanup;
    void *GTY((skip)) user_data;
};

/* Linked list for traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_code"))) lang_struct {
    int lang_code;
    union data_union lang_data;
    struct gcc_string lang_name;
};

/* Array of pointers */
struct GTY(()) pointer_array_container {
    struct basic_struct *GTY((skip)) struct_ptrs[8];
    int *GTY((skip)) int_ptrs[4];
    void (*GTY((skip)) func_ptrs[2])(void);
};

/* Mixed container with all types */
struct GTY(()) mixed_container {
    /* Scalar types */
    int counter;
    double ratio;
    enum color current_color;
    
    /* Struct types */
    struct basic_struct basic;
    struct complex_struct *GTY((skip)) complex_ptr;
    
    /* Union */
    union data_union data;
    
    /* Arrays */
    int numbers[5];
    struct basic_struct items[3];
    
    /* Pointers */
    int *GTY((skip)) dynamic_array;
    struct gcc_string *GTY((skip)) string_ptr;
    
    /* Callback */
    comparator_t GTY((skip)) sorter;
    
    /* Language struct */
    struct lang_struct lang;
    
    /* Pointer arrays */
    struct pointer_array_container *GTY((skip)) ptr_arrays;
    
    /* Linked list */
    struct list_node *GTY((skip)) head;
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct mixed_container main;
    struct user_struct user;
    struct callback_container callbacks[2];
    union data_union unions[3];
    struct gcc_string strings[4];
    
    /* Array of various pointers */
    void *GTY((skip)) void_ptrs[10];
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Nested struct with array of structs */
    struct {
        int count;
        struct basic_struct entries[5];
    } GTY((tag("0"))) nested;
};

/* External declarations for undefined types */
extern struct undefined_struct *global_undefined;

/* Function pointer typedefs for callbacks */
typedef void (*traversal_callback_t)(struct root_container *);
typedef int (*validation_func_t)(const struct mixed_container *);

#endif /* GTY_TEST_H */
