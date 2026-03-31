#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String type */
typedef const char *string_type GTY(());

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef callback_t callback_type GTY(());

/* TYPE_STRUCT: Basic struct type */
struct GTY(()) base_struct {
    scalar_int id;
    string_type name;
    struct opaque *unknown;  /* TYPE_UNDEFINED reference */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int as_int;
    scalar_double as_double;
    struct base_struct *as_struct;
    string_type as_string;
};

/* TYPE_ARRAY: Array type */
typedef struct base_struct *struct_array[10];
typedef struct_array array_type GTY(());

/* TYPE_POINTER: Pointer type */
typedef struct base_struct *struct_ptr;
typedef struct_ptr pointer_type GTY(());

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    scalar_int user_id;
    string_type user_data;
    callback_type user_callback;  /* TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union *value;  /* TYPE_UNION */
    struct lang_tree_node *left;
    struct lang_tree_node *right;
    struct lang_tree_node *parent;
};

/* TYPE_STRUCT with complex nesting */
struct GTY(()) nested_struct {
    struct base_struct base;
    union data_union data;
    struct_array refs;  /* TYPE_ARRAY */
    struct nested_struct *next;  /* TYPE_POINTER */
    struct user_defined_struct *user;  /* TYPE_USER_STRUCT pointer */
    struct lang_tree_node *lang_node;  /* TYPE_LANG_STRUCT pointer */
    callback_type handler;  /* TYPE_CALLBACK */
};

/* Complete the TYPE_UNDEFINED type */
struct GTY(()) opaque {
    scalar_int magic;
    struct nested_struct *owner;
};

/* Top-level complex structure containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_SCALAR */
    scalar_int version;
    scalar_double weight;
    
    /* TYPE_STRING */
    string_type description;
    
    /* TYPE_STRUCT */
    struct base_struct base_item;
    
    /* TYPE_UNION */
    union data_union current_data;
    
    /* TYPE_ARRAY */
    struct_array items;
    
    /* TYPE_POINTER */
    struct nested_struct *nested;
    
    /* TYPE_USER_STRUCT */
    struct user_defined_struct *user_data;
    
    /* TYPE_LANG_STRUCT */
    struct lang_tree_node *ast_root;
    
    /* TYPE_CALLBACK */
    callback_type notify;
    
    /* TYPE_UNDEFINED (now defined) */
    struct opaque *hidden;
    
    /* Self-referential pointer */
    struct top_level *next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with array of pointers */
extern GTY(()) struct top_level *global_roots[5];

#endif /* TEST_GTY_H */
