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
    char * GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void * GTY((skip)) arg);

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unannotated_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    scalar_int_t id;
    scalar_double_t value;
    struct gcc_string * GTY((tag("0"))) name;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    char * GTY((skip)) user_data;
    callback_t user_callback;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char * GTY((skip)) string_val;
    void * GTY((skip)) ptr_val;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct basic_struct * GTY((skip)) struct_ptr_array[10];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_struct_ptr;
typedef union data_union *data_union_ptr;
typedef callback_t *callback_ptr_t;

/* Linked list structure for traversal */
struct GTY(()) list_node {
    int GTY((skip)) data;
    struct list_node * GTY((tag("0"))) next;
    struct list_node * GTY((tag("1"))) prev;
};

/* Complex nested structure */
struct GTY(()) container {
    /* Scalar members */
    scalar_int_t count;
    enum color bg_color;
    
    /* Pointer members */
    struct basic_struct * GTY((tag("0"))) basic;
    union data_union * GTY((tag("1"))) union_data;
    
    /* Array members */
    vec4_t coordinates;
    struct_ptr_array pointers;
    
    /* String member */
    struct gcc_string description;
    
    /* Callback member */
    callback_t notify;
    
    /* Union member */
    union data_union optional_data;
    
    /* Linked structure */
    struct list_node * GTY((tag("2"))) head;
    struct list_node * GTY((tag("3"))) tail;
    
    /* Reference to non-annotated type */
    struct unannotated_struct * GTY((skip)) unannotated_ref;
    
    /* User struct */
    struct user_struct * GTY((tag("4"))) user;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    void * GTY((skip)) lang_data;
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct container main_container;
    struct lang_struct * GTY((tag("0"))) lang_info;
    struct gcc_string root_name;
    
    /* Array of containers */
    struct container containers[5];
    
    /* Pointer array */
    struct basic_struct * GTY((skip)) struct_ptrs[8];
    
    /* Union array */
    union data_union unions[3];
    
    /* Mixed pointer */
    void * GTY((skip)) generic_ptr;
};

/* External declaration to force type inclusion */
extern struct root_container * GTY((skip)) global_root;

#endif /* GTY_TEST_H */
