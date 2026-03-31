#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR and TYPE_STRING */
typedef int scalar_type;
typedef const char *string_type;

/* For TYPE_CALLBACK */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* For TYPE_STRUCT with various members */
struct GTY(()) base_struct {
    scalar_type id;                     /* TYPE_SCALAR */
    string_type name;                   /* TYPE_STRING */
    callback_t callback_func;           /* TYPE_CALLBACK */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    struct base_struct * GTY((tag("0"))) base_ptr;
    int GTY((tag("1"))) int_value;
    double GTY((tag("2"))) double_value;
    string_type GTY((tag("3"))) string_value;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];                /* Fixed-size array */
    struct base_struct * GTY((length("len"))) dyn_array;
    int len;
};

/* For TYPE_POINTER and nested structures */
struct GTY(()) nested_struct {
    struct base_struct * GTY((skip)) direct_ptr;    /* TYPE_POINTER */
    struct opaque * GTY((skip)) opaque_ptr;         /* TYPE_POINTER to undefined */
    union data_union data;                          /* TYPE_UNION */
    int matrix[5][5];                               /* Multi-dimensional array */
};

/* For TYPE_LANG_STRUCT - mimicking GCC language-specific structure */
struct GTY(()) lang_tree_node {
    enum { NODE_TYPE_A, NODE_TYPE_B } node_type;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
    union {
        int int_value;
        double double_value;
        string_type string_value;
    } GTY((desc("%0.node_type"))) u;
};

/* Complete the previously opaque type for TYPE_UNDEFINED -> TYPE_STRUCT transition */
struct GTY(()) opaque {
    int revealed_data;
    struct nested_struct * GTY((skip)) link;
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Basic types */
    scalar_type count;                              /* TYPE_SCALAR */
    string_type description;                        /* TYPE_STRING */
    
    /* Structured types */
    struct base_struct base;                        /* TYPE_STRUCT */
    struct nested_struct nested;                    /* TYPE_STRUCT */
    struct array_container arrays;                  /* TYPE_STRUCT */
    struct user_struct user;                        /* TYPE_USER_STRUCT */
    struct lang_tree_node * GTY((skip)) lang_node;  /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Union type */
    union data_union current_data;                  /* TYPE_UNION */
    
    /* Pointer types */
    struct opaque * GTY((skip)) opaque_ptr;         /* TYPE_POINTER */
    struct top_level * GTY((skip)) self_ptr;        /* TYPE_POINTER (recursive) */
    void * GTY((skip)) generic_ptr;                 /* TYPE_POINTER to void */
    
    /* Array types */
    struct base_struct * GTY((length("num_bases"))) base_array; /* TYPE_ARRAY */
    int num_bases;
    
    /* Callback types */
    callback_t simple_callback;                     /* TYPE_CALLBACK */
    complex_callback_t complex_callback;            /* TYPE_CALLBACK */
    
    /* Nested array of pointers */
    struct nested_struct * GTY((length("nested_count"))) nested_ptr_array[3];
    int nested_count;
};

/* Root variable for gengtype to start processing */
extern GTY(()) struct top_level *global_root;

/* Additional global to ensure processing */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
