#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(struct opaque *, double);

/* For TYPE_STRUCT with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK */
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int numbers[10];           /* Fixed-size array */
    struct base_struct *items[5]; /* Array of pointers */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    struct base_struct *base_ptr;
    struct array_container *array_ptr;
    scalar_int as_int;
    scalar_double as_double;
    callback_t as_callback;
};

/* For TYPE_USER_STRUCT - marked with user attribute */
struct GTY((user)) user_defined_struct {
    int user_data;
    char *user_string;
};

/* For TYPE_LANG_STRUCT - mimicking GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *left;
    struct lang_tree_node *right;
    union data_union *data;
    struct opaque *opaque_ref;  /* TYPE_UNDEFINED reference */
};

/* For TYPE_POINTER - complex pointer relationships */
struct GTY(()) pointer_network {
    struct base_struct **double_ptr;      /* Pointer to pointer */
    union data_union *union_ptr;
    struct array_container *array_ptr;
    struct lang_tree_node *lang_ptr;
    struct user_defined_struct *user_ptr; /* TYPE_USER_STRUCT pointer */
};

/* For TYPE_STRING */
struct GTY(()) string_container {
    const char *constant_string;  /* TYPE_STRING */
    char *dynamic_string;
};

/* Complete the opaque type definition (was TYPE_UNDEFINED, now TYPE_STRUCT) */
struct GTY(()) opaque {
    int revealed_data;
    struct pointer_network *network;
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_POINTER members */
    struct pointer_network *network;
    struct lang_tree_node *lang_root;
    struct user_defined_struct *user_data;
    
    /* TYPE_ARRAY members */
    struct array_container arrays[3];
    callback_t callbacks[4];  /* Array of TYPE_CALLBACK */
    
    /* TYPE_STRING members */
    struct string_container strings;
    
    /* TYPE_SCALAR members */
    scalar_int counter;
    scalar_double total;
    
    /* Reference to previously undefined type */
    struct opaque *opaque_ptr;
    
    /* Nested structure */
    struct GTY(()) {
        int nested_id;
        struct top_level *parent;  /* Recursive pointer */
    } nested;
    
    /* Flexible array member */
    struct base_struct *flex_array GTY((length("flex_count")));
    int flex_count;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional global to ensure all types are referenced */
extern GTY(()) struct lang_tree_node *global_lang_node;

#endif /* TEST_GTY_H */
