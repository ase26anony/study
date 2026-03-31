#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/opaque type */
struct opaque;

/* Basic scalar types */
typedef int my_scalar_t;
typedef double my_double_t;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int param);
typedef void (*complex_callback_t)(struct opaque*, int);

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void* user_ptr;
};

/* For TYPE_STRUCT with various members */
struct GTY(()) base_struct {
    int scalar_field;              /* TYPE_SCALAR */
    double double_field;           /* TYPE_SCALAR */
    char* string_field;            /* TYPE_STRING */
    struct opaque* opaque_ptr;     /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    double double_val;
    struct base_struct* struct_ptr;
    char* string_ptr;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];           /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct* ptr_array[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* Nested structure with complex members */
struct GTY(()) nested_struct {
    struct base_struct* base;      /* TYPE_POINTER to TYPE_STRUCT */
    union data_union data;         /* TYPE_UNION */
    struct nested_struct* next;    /* TYPE_POINTER (linked list) */
    callback_t callback;           /* TYPE_CALLBACK */
    int dynamic_array GTY((length("dynamic_len")));
    int dynamic_len;
};

/* For TYPE_LANG_STRUCT - mimicking GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node* child;
        struct base_struct* base;
    } GTY((desc("code"))) u;
    struct lang_tree_node* chain;
};

/* Top-level structure containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;               /* TYPE_STRUCT */
    struct nested_struct* nested;          /* TYPE_POINTER */
    union data_union union_field;          /* TYPE_UNION */
    struct array_container arrays;         /* TYPE_STRUCT containing TYPE_ARRAY */
    
    /* Various pointer types */
    struct opaque* undefined_ptr;          /* TYPE_POINTER to TYPE_UNDEFINED */
    struct user_struct* user_struct_ptr;   /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct lang_tree_node* lang_struct_ptr; /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Direct scalar and string types */
    int scalar_value;                      /* TYPE_SCALAR */
    char* string_value;                    /* TYPE_STRING */
    
    /* Callback function */
    callback_t callback_func;              /* TYPE_CALLBACK */
    complex_callback_t complex_callback;   /* TYPE_CALLBACK */
    
    /* Array types */
    int int_array[20];                     /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct* struct_ptr_array[8]; /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Self-referential pointer */
    struct top_level* self_ptr;            /* TYPE_POINTER */
    
    /* Union with tag for discrimination */
    union {
        int as_int;
        struct nested_struct* as_nested;
    } GTY((tag("union_tag"))) tagged_union;
    int union_tag;
};

/* Complete the previously undefined struct */
struct opaque {
    struct top_level* top;
    int hidden_data;
};

/* Root variable for gengtype to process */
extern GTY(()) struct top_level* global_root;

/* Additional global variables to ensure processing */
extern GTY(()) struct lang_tree_node* lang_root;
extern GTY(()) struct user_struct* user_global;
extern GTY(()) union data_union global_union;

#endif /* TEST_GTY_H */
