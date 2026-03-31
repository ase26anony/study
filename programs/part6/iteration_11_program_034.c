/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (handled as scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *data, int param);

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct plain_struct {
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
    int user_id;
    void *GTY((skip)) user_data;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;
    struct basic_struct *GTY((skip)) struct_ptr;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef struct basic_struct *GTY((skip)) struct_ptr_array[10];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef callback_t callback_ptr_t;

/* Linked list structure for chained references */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
};

/* Complex nested structure */
struct GTY(()) complex_struct {
    /* Scalar members */
    scalar_int_t count;
    scalar_double_t total;
    enum color color;
    
    /* String type */
    struct gcc_string GTY((skip)) description;
    
    /* Callback type */
    callback_t GTY((skip)) handler;
    
    /* Union type */
    union data_union GTY((skip)) variant;
    
    /* Array types */
    vec4_t coordinates;
    struct_ptr_array pointers;
    
    /* Pointer types */
    struct basic_struct *GTY((skip)) basic_ptr;
    struct user_struct *GTY((skip)) user_ptr;
    int_ptr_t int_ptr;
    
    /* Nested struct */
    struct {
        int nested_id;
        char nested_name[32];
    } GTY((skip)) nested;
    
    /* Linked list */
    struct list_node *GTY((skip)) head;
    struct list_node *GTY((skip)) tail;
    
    /* Reference to non-annotated type */
    struct plain_struct *GTY((skip)) plain_ptr;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_type"), tag("LANG_STRUCT"))) lang_struct {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Root structure containing all types */
struct GTY(()) root_container {
    /* Various struct types */
    struct basic_struct GTY((skip)) basic;
    struct user_struct GTY((skip)) user;
    struct complex_struct GTY((skip)) complex;
    struct lang_struct GTY((skip)) lang;
    
    /* Direct scalar */
    int root_scalar;
    
    /* String */
    struct gcc_string GTY((skip)) root_string;
    
    /* Union */
    union data_union GTY((skip)) root_union;
    
    /* Arrays */
    struct basic_struct GTY((skip)) struct_array[5];
    int int_array[20];
    
    /* Pointers to everything */
    struct basic_struct *GTY((skip)) ptr_to_basic;
    struct user_struct *GTY((skip)) ptr_to_user;
    struct complex_struct *GTY((skip)) ptr_to_complex;
    struct lang_struct *GTY((skip)) ptr_to_lang;
    struct gcc_string *GTY((skip)) ptr_to_string;
    struct plain_struct *GTY((skip)) ptr_to_plain;
    callback_t *GTY((skip)) ptr_to_callback;
    
    /* Callback */
    callback_t GTY((skip)) root_callback;
    
    /* Self-referential pointer for graph traversal */
    struct root_container *GTY((skip)) next_root;
};

/* Global variable marked for GC */
extern struct root_container *GTY((root)) global_root;

#endif /* GTY_TEST_H */
