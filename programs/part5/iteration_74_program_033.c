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

/* For TYPE_STRUCT with nested types */
struct GTY(()) base_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    double double_field;
    
    /* TYPE_STRING */
    const char *string_field;
    
    /* TYPE_POINTER to undefined type */
    struct opaque *opaque_ptr;
    
    /* TYPE_CALLBACK */
    callback_t callback_field;
};

/* For TYPE_UNION */
union GTY(()) data_union {
    /* TYPE_POINTER */
    struct base_struct *struct_ptr;
    
    /* TYPE_SCALAR */
    int int_value;
    double double_value;
    
    /* TYPE_STRING */
    const char *string_value;
    
    /* TYPE_ARRAY (inline) */
    int small_array[10];
};

/* For TYPE_ARRAY */
typedef struct GTY(()) array_element {
    int id;
    struct base_struct * GTY((skip)) next;  /* Skip for GC, but still processed */
} element_array[100];

/* For TYPE_LANG_STRUCT - mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc ("0"))) u;
    struct lang_tree_node * GTY((skip)) chain;
    struct lang_tree_node * GTY((tag ("1"))) left;
    struct lang_tree_node * GTY((tag ("2"))) right;
};

/* For nested TYPE_POINTER and complex relationships */
struct GTY(()) nested_struct {
    /* TYPE_POINTER to another struct */
    struct base_struct *base_ptr;
    
    /* TYPE_POINTER to union */
    union data_union *union_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct base_struct * GTY((length ("array_len"))) ptr_array[50];
    int array_len;
    
    /* TYPE_ARRAY of scalars */
    int int_array[100];
    
    /* TYPE_CALLBACK with complex signature */
    complex_callback_t complex_callback;
    
    /* TYPE_POINTER to user struct */
    struct user_struct *user_ptr;
};

/* Complete the previously undefined TYPE_UNDEFINED */
struct GTY(()) opaque {
    int revealed_data;
    struct nested_struct *nested;
    struct lang_tree_node *lang_node;
};

/* Top-level structure containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT nested */
    struct base_struct base;
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_POINTER */
    struct nested_struct *nested;
    
    /* TYPE_ARRAY */
    element_array elements;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_tree_node *lang_struct;
    
    /* TYPE_USER_STRUCT */
    struct user_struct user;
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_CALLBACK */
    callback_t handlers[5];
    
    /* TYPE_POINTER to opaque (now defined) */
    struct opaque *opaque_obj;
    
    /* Self-referential TYPE_POINTER */
    struct top_level * GTY((skip)) next;
    
    /* TYPE_ARRAY of unions */
    union data_union union_array[20];
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with array type */
extern GTY(()) element_array global_array;

/* Root with callback type */
extern GTY(()) callback_t global_callback;

#endif /* TEST_GTY_H */
