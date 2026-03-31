#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/forward declaration */
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

/* For TYPE_STRUCT with nested types */
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
    string_type GTY((tag("3"))) str_value;
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
        int int_val;
        double double_val;
        string_type str_val;
    } GTY((desc("node_type"))) value;
};

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    struct nested_struct * GTY((skip)) nested;
    struct user_struct * GTY((skip)) user;
    struct lang_tree_node * GTY((skip)) lang_node;
};

/* Top-level complex structure hitting all type cases */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;            /* TYPE_STRUCT */
    struct nested_struct nested;        /* TYPE_STRUCT */
    
    /* TYPE_POINTER members */
    struct opaque * GTY((skip)) opaque_ptr;     /* TYPE_POINTER */
    struct user_struct * GTY((skip)) user_ptr;  /* TYPE_POINTER to USER_STRUCT */
    
    /* TYPE_UNION member */
    union data_union current_data;      /* TYPE_UNION */
    
    /* TYPE_ARRAY members */
    struct array_container arrays;      /* Contains TYPE_ARRAY */
    callback_t callback_array[5];       /* Array of TYPE_CALLBACK */
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_tree_node * GTY((skip)) lang_tree;  /* TYPE_POINTER to LANG_STRUCT */
    
    /* String array */
    string_type GTY((length("string_count"))) strings[10];
    int string_count;
    
    /* Scalar types */
    scalar_type scalar_field;           /* TYPE_SCALAR */
    float float_field;
    double double_field;
    
    /* Complex callback */
    complex_callback_t complex_callback;  /* TYPE_CALLBACK */
    
    /* Self-referential pointer */
    struct top_level * GTY((skip)) next;  /* TYPE_POINTER */
};

/* Root variable for gengtype to start processing */
extern GTY(()) struct top_level *global_root;

/* Additional root variables to ensure all types are processed */
extern GTY(()) struct user_struct *global_user_struct;
extern GTY(()) struct lang_tree_node *global_lang_tree;
extern GTY(()) union data_union global_union;

#endif /* TEST_GTY_H */
