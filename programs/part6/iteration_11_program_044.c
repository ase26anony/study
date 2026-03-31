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

/* TYPE_UNDEFINED: Non-GTY type referenced by GTY types */
struct undefined_helper {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((tag("0"))) base;
    struct undefined_helper *helper;  /* Non-GTY pointer */
    int flags;
    enum color color;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *GTY((skip)) opaque_data;
    int user_id;
    char *GTY((length("strlen($) + 1"))) username;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((tag("1"))) string_val;
    void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct *struct_ptr_array[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int GTY((skip)) length;
    char *GTY((length("strlen($) + 1"))) data;
    const char *GTY((length("strlen($) + 1"))) const_data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*event_handler_t)(int event_id, void *user_data);

/* TYPE_CALLBACK in struct */
struct GTY(()) event_manager {
    event_handler_t GTY((skip)) handler;
    void *GTY((skip)) user_data;
    int last_event;
};

/* Linked list for traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((tag("0"))) next;
    struct list_node *GTY((tag("1"))) prev;
};

/* Array of unions */
struct GTY(()) union_container {
    union data_union items[8];
    int count;
};

/* Mixed type container */
struct GTY(()) type_container {
    /* Scalar members */
    int id;
    enum color color;
    double weight;
    
    /* Pointer members */
    struct basic_struct *GTY((tag("0"))) basic_ptr;
    struct complex_struct *GTY((tag("1"))) complex_ptr;
    
    /* Array members */
    int scores[5];
    struct gcc_string strings[3];
    
    /* Union member */
    union data_union current_data;
    
    /* Callback member */
    comparator_t GTY((skip)) compare_func;
    
    /* String member */
    struct gcc_string name;
    
    /* Linked list */
    struct list_node *GTY((tag("2"))) head;
    
    /* Array of pointers */
    struct user_struct *GTY((tag("3"))) users[4];
    
    /* Non-GTY reference */
    struct undefined_helper *helper;
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct type_container main_container;
    struct event_manager events;
    struct union_container unions;
    struct gcc_string title;
    
    /* Various pointers to ensure traversal */
    struct root_container *GTY((tag("0"))) self_ptr;
    struct root_container *GTY((tag("1"))) *ptr_array[2];
    
    /* Callback array */
    event_handler_t GTY((skip)) handlers[3];
    
    /* Mixed array */
    void *GTY((skip)) mixed_array[4];
};

/* Additional non-GTY types for undefined references */
typedef struct {
    int a, b, c;
} triple_t;

struct external_ref {
    triple_t triple;
    char *description;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_type {
    struct lang_type *next;
    int lang_specific_data;
};
#endif

/* Forward declarations to create complex type graph */
struct GTY(()) forward_decl;
struct GTY(()) another_forward;

struct GTY(()) forward_decl {
    int data;
    struct another_forward *GTY((tag("0"))) link;
};

struct GTY(()) another_forward {
    char label[16];
    struct forward_decl *GTY((tag("0"))) backlink;
    struct forward_decl *GTY((tag("1"))) *ptr_array[2];
};

#endif /* GTY_TEST_H */
