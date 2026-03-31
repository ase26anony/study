#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int data[10];
    struct base_struct *ptr_array[5];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *base_ptr;
    struct array_container *array_ptr;
    scalar_int as_int;
    scalar_double as_double;
};

/* TYPE_POINTER: Pointer-only struct */
struct GTY(()) pointer_chain {
    struct pointer_chain *next;
    struct base_struct *data;
    void *generic_ptr;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_defined_struct {
    int user_data;
    char *user_string;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union *lang_data;
    struct lang_tree_node *lang_next;
    struct lang_tree_node *lang_children[2];
};

/* Complete the TYPE_UNDEFINED opaque struct */
struct GTY(()) opaque {
    struct base_struct *revealed;
    struct user_defined_struct *user;
};

/* TYPE_STRING: String pointer in a struct */
struct GTY(()) string_container {
    const char *GTY((tag("0"))) static_string;
    char *dynamic_string;
    const char *array_of_strings[3];
};

/* Complex nested struct exercising multiple type kinds */
struct GTY(()) nested_complex {
    /* TYPE_STRUCT member */
    struct base_struct base;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_ARRAY members */
    struct pointer_chain *chain_array[4];
    callback_t callback_array[2];
    
    /* TYPE_POINTER members */
    struct array_container *array_ptr;
    struct user_defined_struct *user_ptr;
    struct opaque *opaque_ptr;
    
    /* TYPE_CALLBACK member */
    callback_t handler;
    complex_callback_t complex_handler;
    
    /* TYPE_STRING members */
    const char *name;
    struct string_container *strings;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_tree_node *lang_node;
    
    /* Self-referential pointer */
    struct nested_complex *self;
    
    /* Scalar types */
    scalar_int count;
    scalar_double precision;
};

/* Top-level root structure */
struct GTY(()) top_level {
    struct nested_complex *complex;
    struct lang_tree_node *tree_root;
    union data_union root_data;
    struct user_defined_struct *user_data;
    struct string_container *all_strings;
    struct opaque *opaque_data;
    
    /* Array of various types */
    struct base_struct *struct_array[8];
    callback_t callbacks[4];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structures */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
