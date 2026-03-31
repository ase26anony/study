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
    struct nested_struct * GTY((chain_next("%h.next"))) next;  /* TYPE_POINTER with chain */
    union data_union data;                          /* TYPE_UNION */
    int matrix[5][5];                               /* Multi-dimensional array */
};

/* For TYPE_LANG_STRUCT - mimicking GCC language-specific structure */
struct GTY(()) lang_tree_node {
    enum tree_code code;
    union lang_tree_union {
        struct lang_tree_node * GTY((tag("0"))) child;
        long GTY((tag("1"))) int_cst;
        double GTY((tag("2"))) real_cst;
    } GTY((desc("(%h.code)"))) u;
    struct lang_tree_node * GTY((skip)) chain;
};

/* Complete the opaque type definition */
struct GTY(()) opaque {
    struct nested_struct * GTY((reorder("resort_nested"))) nested;
    struct lang_tree_node *lang_node;
    struct user_struct *user;           /* TYPE_USER_STRUCT pointer */
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Basic types */
    scalar_type count;                  /* TYPE_SCALAR */
    string_type description;            /* TYPE_STRING */
    
    /* Pointer types */
    struct opaque *opaque_ptr;          /* TYPE_POINTER */
    struct base_struct **ptr_array;     /* TYPE_POINTER to TYPE_ARRAY of pointers */
    
    /* Structure types */
    struct base_struct base;            /* TYPE_STRUCT */
    struct nested_struct nested;        /* TYPE_STRUCT with nested types */
    
    /* Union type */
    union data_union current_data;      /* TYPE_UNION */
    
    /* Array types */
    struct array_container arrays;      /* TYPE_STRUCT containing TYPE_ARRAY */
    int multi_dim[3][4][5];             /* Multi-dimensional TYPE_ARRAY */
    
    /* Language structure */
    struct lang_tree_node *lang_root;   /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* User structure */
    struct user_struct user_data;       /* TYPE_USER_STRUCT */
    
    /* Callback function */
    complex_callback_t notify;          /* TYPE_CALLBACK */
    
    /* Self-referential pointer for graph traversal */
    struct top_level * GTY((skip)) next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional global to ensure processing */
extern GTY(()) struct opaque *global_opaque;

#endif /* TEST_GTY_H */
