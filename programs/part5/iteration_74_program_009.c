#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR and basic types */
typedef int scalar_type;
typedef double double_type;

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
    int id;                          /* TYPE_SCALAR */
    char * GTY((skip)) name;         /* TYPE_STRING (skip annotation for string) */
    struct opaque *unknown_ptr;      /* TYPE_UNDEFINED pointer */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int as_int;                      /* TYPE_SCALAR */
    double as_double;                /* TYPE_SCALAR */
    struct base_struct * GTY((tag("0"))) as_struct;  /* TYPE_POINTER with tag */
    void *as_pointer;                /* TYPE_POINTER */
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];             /* TYPE_ARRAY of scalars */
    struct base_struct *ptr_array[5]; /* TYPE_ARRAY of pointers */
};

/* For nested structures and TYPE_POINTER */
struct GTY(()) nested_struct {
    struct base_struct *parent;      /* TYPE_POINTER to struct */
    union data_union data;           /* TYPE_UNION */
    struct nested_struct *next;      /* TYPE_POINTER to same type (linked list) */
    struct nested_struct *prev;      /* TYPE_POINTER to same type */
    int matrix[3][3];                /* Multi-dimensional TYPE_ARRAY */
};

/* For TYPE_LANG_STRUCT - mimicking GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node * GTY((chain_next("%h.next"))) next;
    struct lang_tree_node *children[2];
    union data_union value;
};

/* For TYPE_STRING */
struct GTY(()) string_container {
    const char * GTY((length("strlen(%h.ptr)+1"))) ptr;  /* TYPE_STRING with length */
    char *dynamic_str;               /* TYPE_STRING */
};

/* Complete the opaque type definition */
struct GTY(()) opaque {
    int revealed;
    struct nested_struct *link;
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;         /* TYPE_STRUCT */
    struct nested_struct nested;     /* TYPE_STRUCT */
    
    /* TYPE_UNION */
    union data_union union_data;     /* TYPE_UNION */
    
    /* TYPE_POINTER variations */
    struct opaque *opaque_ptr;       /* TYPE_POINTER to now-defined opaque */
    struct user_struct *user_ptr;    /* TYPE_POINTER to user struct */
    struct lang_tree_node *lang_ptr; /* TYPE_POINTER to lang struct */
    
    /* TYPE_ARRAY variations */
    int scalar_array[20];            /* TYPE_ARRAY of scalars */
    struct base_struct *struct_ptr_array[8]; /* TYPE_ARRAY of pointers */
    
    /* TYPE_CALLBACK */
    callback_t simple_callback;      /* TYPE_CALLBACK */
    complex_callback_t complex_callback; /* TYPE_CALLBACK with struct param */
    
    /* TYPE_STRING */
    struct string_container strings; /* TYPE_STRUCT containing strings */
    
    /* For variable length array simulation */
    int flexible_array GTY((length("%h.dynamic_len")))[];
    int dynamic_len;
    
    /* TYPE_USER_STRUCT embedded */
    struct user_struct user_instance; /* TYPE_USER_STRUCT */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Another root with different structure for more coverage */
extern GTY(()) struct lang_tree_node *lang_root;

#endif /* TEST_GTY_H */
