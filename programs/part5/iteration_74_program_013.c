#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/forward declaration */
struct opaque;

/* For TYPE_SCALAR and basic types */
typedef int my_scalar_t;
typedef double my_double_t;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void *user_ptr;
};

/* For TYPE_STRUCT with various members */
struct GTY(()) base_struct {
    int scalar_field;              /* TYPE_SCALAR */
    double double_field;           /* TYPE_SCALAR */
    char * GTY((skip)) string_ptr; /* TYPE_STRING - skip prevents recursion */
    struct opaque *opaque_ptr;     /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int as_int;
    double as_double;
    struct base_struct * GTY((tag("0"))) as_struct;
    void *as_pointer;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];           /* TYPE_ARRAY fixed size */
    struct base_struct *var_array[5]; /* TYPE_ARRAY of pointers */
    int *dynamic_array;            /* TYPE_POINTER (could be array) */
};

/* For TYPE_LANG_STRUCT - mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("code"))) u;
    struct lang_tree_node *chain;
};

/* For nested structures with pointers */
struct GTY(()) nested_struct {
    struct base_struct inner;
    union data_union data;
    struct nested_struct *next;    /* TYPE_POINTER to same type */
    struct nested_struct *prev;    /* TYPE_POINTER to same type */
};

/* For complex pointer chains */
struct GTY(()) pointer_chain {
    struct base_struct **double_ptr;    /* TYPE_POINTER to TYPE_POINTER */
    struct nested_struct *nested_ptr;
    union data_union *union_ptr;
};

/* Top-level structure containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;            /* TYPE_STRUCT */
    struct user_struct *user;           /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* TYPE_UNION */
    union data_union data;              /* TYPE_UNION */
    
    /* TYPE_ARRAY */
    struct array_container arrays;      /* TYPE_STRUCT containing arrays */
    int matrix[3][4];                   /* Multi-dimensional TYPE_ARRAY */
    
    /* TYPE_POINTER variations */
    struct opaque *forward_ptr;         /* TYPE_POINTER to TYPE_UNDEFINED */
    struct lang_tree_node *lang_ptr;    /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct pointer_chain *chain_ptr;    /* TYPE_POINTER to complex struct */
    
    /* TYPE_CALLBACK */
    callback_t simple_callback;         /* TYPE_CALLBACK */
    complex_callback_t complex_callback; /* TYPE_CALLBACK */
    
    /* TYPE_SCALAR */
    my_scalar_t scalar_type;            /* TYPE_SCALAR via typedef */
    
    /* TYPE_STRING */
    const char * GTY((skip)) message;   /* TYPE_STRING */
    
    /* Self-referential pointer */
    struct top_level *self;             /* TYPE_POINTER to same type */
};

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    struct top_level *connection;
    int hidden_data;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with array */
extern GTY(()) struct top_level *global_array_root[5];

#endif /* TEST_GTY_H */
