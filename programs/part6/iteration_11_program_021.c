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
typedef int (*callback_t)(void *data, int param);

/* Non-annotated struct (may become TYPE_UNDEFINED when referenced) */
struct unannotated {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    int id;
    char *GTY((skip)) name;
    double value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int magic;
    void *GTY((skip)) user_data;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;
    struct basic_struct *GTY((ptr)) struct_ptr;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct basic_struct *GTY((ptr)) struct_ptr_array[10];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY((ptr)) basic_ptr_t;
typedef void *GTY((ptr)) generic_ptr_t;
typedef callback_t callback_ptr_t;

/* Linked list structure (for traversal) */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((ptr)) next;
    struct list_node *GTY((ptr)) prev;
};

/* Complex nested structure */
struct GTY(()) complex_struct {
    /* Scalar members */
    scalar_int_t count;
    scalar_double_t total;
    enum color color;
    
    /* Pointer members */
    struct basic_struct *GTY((ptr)) basic;
    struct user_struct *GTY((ptr)) user;
    struct unannotated *unannotated_ptr;  /* No GTY marker */
    
    /* Union member */
    union data_union storage;
    
    /* Array members */
    int scores[5];
    vec4_t vector;
    struct_ptr_array ptrs;
    
    /* String member */
    struct gcc_string description;
    
    /* Callback member */
    callback_t handler;
    
    /* Nested struct */
    struct {
        int nested_id;
        char nested_name[32];
    } GTY((tag("0"))) nested;
    
    /* Pointer to array */
    int (*GTY((ptr)) matrix)[4];
    
    /* Self-referential pointer */
    struct complex_struct *GTY((ptr)) sibling;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%1.type"), tag("0"))) lang_struct {
    enum {
        LANG_INT,
        LANG_FLOAT,
        LANG_STRING
    } type;
    
    union {
        int int_val;
        double float_val;
        struct gcc_string *GTY((ptr)) string_val;
    } GTY((desc("%0.type"))) value;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    struct basic_struct *GTY((ptr)) basic_item;
    struct user_struct *GTY((ptr)) user_item;
    union data_union *GTY((ptr)) union_item;
    struct list_node *GTY((ptr)) list_head;
    struct complex_struct *GTY((ptr)) complex_item;
    struct lang_struct *GTY((ptr)) lang_item;
    struct gcc_string *GTY((ptr)) string_item;
    
    /* Array of various pointers */
    void *GTY((ptr)) generic_items[8];
    
    /* Callback array */
    callback_t callbacks[3];
    
    /* Direct embedded struct */
    struct basic_struct embedded_basic;
    
    /* Pointer to function pointer */
    callback_t *callback_ptr;
};

/* External variable declarations for root marking */
extern struct root_container GTY((root)) global_root;

#endif /* GTY_TEST_H */
