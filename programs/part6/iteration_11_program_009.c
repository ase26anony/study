/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((skip)) data;
};

/* TYPE_ARRAY: Array typedef */
typedef int vec4_t[4];
typedef char buffer_t[256];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *);
typedef void (*cleanup_fn)(struct gcc_string *);

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unannotated {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;                     /* scalar */
    char name[32];              /* array */
    struct gcc_string *desc;    /* pointer to string struct */
    vec4_t coordinates;         /* array typedef */
    enum color col;             /* enum scalar */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int magic;
    void * GTY((skip)) opaque_data;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int as_int;
    double as_double;
    char *as_string;
    struct basic_struct *as_struct;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;
typedef int *int_ptr_t;
typedef callback_t callback_ptr_t;

/* Linked list structure (chain of types) */
struct GTY(()) node {
    int data;
    struct node * GTY((skip)) next;
    struct node * GTY((skip)) prev;
};

/* Complex nested structure */
struct GTY(()) container {
    /* Array of pointers */
    struct basic_struct * GTY((length("count"))) items[10];
    int count;
    
    /* Union member */
    union data_union value;
    
    /* Callback function pointer */
    callback_t notify;
    
    /* Pointer to unannotated type */
    struct unannotated *unann;
    
    /* Nested array of arrays */
    int matrix[3][3];
    
    /* String pointer */
    char * GTY((skip)) dynamic_string;
    
    /* User struct */
    struct user_struct user;
    
    /* Self-referential pointer */
    struct container * GTY((skip)) parent;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_flag;
    void * GTY((tag("0"))) lang_data;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_type {
    struct basic_struct *basic;
    union data_union *union_ptr;
    struct node *list_head;
    struct container *container;
    struct lang_struct *lang;
    struct user_struct *user;
    struct gcc_string *title;
    
    /* Array of callbacks */
    callback_t callbacks[5];
    
    /* Pointer to array */
    int * GTY((skip)) dynamic_array;
    
    /* Mixed array */
    void * GTY((skip)) mixed[8];
};

/* Another struct with function pointer table */
struct GTY(()) vtable {
    callback_t func1;
    cleanup_fn func2;
    int (*func3)(struct basic_struct *, int);
};

/* Forward declaration (may be TYPE_UNDEFINED initially) */
struct forward_declared;

/* Struct using forward declared type */
struct GTY(()) uses_forward {
    struct forward_declared * GTY((skip)) fwd_ptr;
    int valid;
};

/* Actually define the forward declared struct */
struct GTY(()) forward_declared {
    int data;
    struct uses_forward *owner;
};

#endif /* GTY_TEST_H */
