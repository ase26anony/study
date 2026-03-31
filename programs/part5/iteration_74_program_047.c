#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/opaque type */
struct opaque;

/* Basic scalar types */
typedef int my_scalar;
typedef double my_double;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* For TYPE_STRUCT with various members */
struct GTY(()) base_struct {
    int scalar_field;              /* TYPE_SCALAR */
    double double_field;           /* TYPE_SCALAR */
    char * GTY((skip)) string_ptr; /* TYPE_STRING (via pointer) */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int as_int;
    double as_double;
    struct base_struct * GTY((tag("0"))) as_struct_ptr;
    void *as_pointer;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];           /* Fixed-size array */
    struct base_struct * GTY((length("5"))) dyn_array[5]; /* Array of pointers */
};

/* For TYPE_POINTER - nested pointer structures */
struct GTY(()) pointer_chain {
    struct base_struct * GTY((skip)) direct_ptr;
    struct pointer_chain * GTY((skip)) next;
    void * GTY((skip)) void_ptr;
};

/* For TYPE_LANG_STRUCT - mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("code"))) u;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
};

/* Complete the previously opaque type for TYPE_UNDEFINED -> TYPE_STRUCT transition */
struct GTY(()) opaque {
    int now_defined;
    struct base_struct * GTY((skip)) connection;
};

/* Complex nested structure hitting multiple type cases */
struct GTY(()) nested_complex {
    /* TYPE_STRUCT members */
    struct base_struct base;           /* Embedded struct */
    
    /* TYPE_POINTER members */
    struct opaque * GTY((skip)) opaque_ptr;
    struct nested_complex * GTY((skip)) self_ptr;
    
    /* TYPE_UNION member */
    union data_union data;             /* Embedded union */
    
    /* TYPE_ARRAY members */
    int matrix[3][3];                  /* Multi-dimensional array */
    callback_t callbacks[5];           /* Array of callbacks */
    
    /* TYPE_CALLBACK member */
    callback_t single_callback;        /* Single function pointer */
    
    /* TYPE_USER_STRUCT pointer */
    struct user_struct * GTY((skip)) user;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_tree_node * GTY((skip)) lang_node;
    
    /* TYPE_STRING pointer */
    const char * GTY((skip)) name;
    
    /* Scalar types */
    unsigned long scalar1;
    short scalar2;
};

/* Top-level structure containing everything */
struct GTY(()) top_level {
    /* Various struct types */
    struct base_struct simple;
    struct nested_complex complex;
    struct pointer_chain * GTY((skip)) chain;
    
    /* Union */
    union data_union choice;
    
    /* Arrays */
    struct nested_complex * GTY((length("3"))) complex_array[3];
    
    /* Opaque pointer (was undefined) */
    struct opaque * GTY((skip)) mystery;
    
    /* Language structure */
    struct lang_tree_node * GTY((skip)) tree_root;
    
    /* User structure */
    struct user_struct * GTY((skip)) user_data;
    
    /* Callbacks */
    callback_t handlers[2];
    complex_callback_t advanced_handler;
    
    /* String */
    const char * GTY((skip)) description;
    
    /* Scalars */
    int counter;
    double precision;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type for more coverage */
extern GTY(()) struct nested_complex *secondary_root;

#endif /* TEST_GTY_H */
