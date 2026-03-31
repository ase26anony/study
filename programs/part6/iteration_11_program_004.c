/* Test header for gengtype-state.cc coverage */
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
    char *data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *data, int value);

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unannotated_struct {
    int x;
    float y;
};

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;                     /* scalar */
    char name[32];             /* array */
    struct gcc_string *desc;   /* pointer to string struct */
    callback_t handler;        /* callback */
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct basic_struct *struct_ptr_array_t[10];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;
typedef int *int_ptr_t;
typedef callback_t *callback_ptr_t;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int as_int;
    double as_double;
    char *as_string;
    struct basic_struct *as_struct;
};

/* Nested struct for more complexity */
struct GTY(()) nested_container {
    struct basic_struct inner;
    union data_union variant;
    vec4_t coordinates;
};

/* Linked list structure (creates pointer chain) */
struct GTY(()) list_node {
    int value;
    struct list_node *next;    /* pointer to same type */
    struct list_node *prev;    /* another pointer */
};

/* Array of unions */
union GTY(()) small_union {
    int i;
    char c;
    float f;
};

struct GTY(()) array_container {
    union small_union items[8];
    struct list_node *node_array[5];
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    void *opaque_data;
    int user_tag;
    struct basic_struct *associated;
};

/* Function pointer with complex signature */
typedef union data_union *(*complex_callback_t)(
    struct basic_struct *context,
    int operation,
    void *user_data
);

/* Struct with function pointer member */
struct GTY(()) callback_container {
    complex_callback_t processor;
    void *user_data;
    int state;
};

/* Mixed array types */
struct GTY(()) mixed_arrays {
    int scalar_array[20];
    struct basic_struct *pointer_array[15];
    union data_union union_array[10];
    callback_t callback_array[5];
};

/* Deeply nested structure */
struct GTY(()) level1 {
    int a;
    struct GTY(()) level2 {
        int b;
        struct GTY(()) level3 {
            int c;
            struct level1 *parent;  /* circular reference */
        } deep;
    } mid;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Basic types */
    scalar_int_t count;
    scalar_double_t total;
    
    /* String */
    struct gcc_string *title;
    
    /* Structs */
    struct basic_struct *basic;
    struct nested_container *nested;
    
    /* Unions */
    union data_union *main_data;
    
    /* Lists */
    struct list_node *head;
    struct list_node *tail;
    
    /* Arrays */
    vec4_t vector;
    struct_ptr_array_t struct_ptrs;
    
    /* Callbacks */
    callback_t simple_callback;
    complex_callback_t complex_callback;
    
    /* User struct */
    struct user_defined_struct *user_data;
    
    /* Array containers */
    struct array_container *arrays;
    struct mixed_arrays *mixed;
    
    /* Nested */
    struct level1 *hierarchy;
    
    /* Callback container */
    struct callback_container *cb_container;
    
    /* Pointer to unannotated type (may trigger special handling) */
    struct unannotated_struct *unannotated;
    
    /* Self-reference for graph complexity */
    struct root_container *next_root;
};

/* Global root variable */
extern struct root_container GTY(()) *global_root;

#endif /* GTY_TEST_H */
