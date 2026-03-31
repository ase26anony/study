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

/* For TYPE_STRING */
typedef const char *string_t;

/* For TYPE_ARRAY */
typedef int int_array_t[10];
typedef struct opaque *opaque_array_t[5];

/* For TYPE_USER_STRUCT - marked with GTY((user)) */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* For TYPE_STRUCT with GTY(()) */
struct GTY(()) base_struct {
    int id;
    double value;
    /* TYPE_STRING member */
    const char * GTY((skip)) name;
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    double double_val;
    struct base_struct * GTY((tag("0"))) base_ptr;
    void * GTY((tag("1"))) generic_ptr;
};

/* For TYPE_POINTER in various forms */
typedef struct base_struct *base_ptr_t;
typedef union data_union *union_ptr_t;

/* Nested struct with multiple pointer types */
struct GTY(()) nested_struct {
    struct base_struct * GTY((skip)) base;      /* TYPE_POINTER */
    union data_union * GTY((skip)) union_data;  /* TYPE_POINTER */
    int_array_t numbers;                         /* TYPE_ARRAY */
    callback_t handler;                          /* TYPE_CALLBACK */
};

/* For TYPE_LANG_STRUCT - mimicking GCC language structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
    union data_union GTY((skip)) value;
};

/* Complete the previously opaque struct for TYPE_UNDEFINED -> TYPE_STRUCT transition */
struct GTY(()) opaque {
    int magic;
    struct nested_struct * GTY((skip)) nested;
    struct lang_tree_node * GTY((skip)) lang_node;
};

/* Complex top-level struct containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_SCALAR members */
    int counter;
    double ratio;
    
    /* TYPE_STRUCT members */
    struct base_struct base;
    struct nested_struct nested;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_POINTER members */
    struct opaque * GTY((skip)) opaque_ptr;
    struct user_struct * GTY((skip)) user_ptr;
    
    /* TYPE_ARRAY members */
    struct base_struct * GTY((skip)) ptr_array[5];
    int matrix[3][4];
    
    /* TYPE_STRING member */
    const char * GTY((skip)) description;
    
    /* TYPE_CALLBACK member */
    complex_callback_t notify;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_tree_node * GTY((skip)) tree_root;
    
    /* Self-referential pointer */
    struct top_level * GTY((skip)) next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with array type */
extern GTY(()) struct top_level *root_array[3];

#endif /* TEST_GTY_H */
