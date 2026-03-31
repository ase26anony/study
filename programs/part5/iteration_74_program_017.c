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
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int numbers[10];           /* Fixed-size array */
    struct base_struct *items[5]; /* Array of pointers */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *gs_ptr;
    struct array_container *array_ptr;
    scalar_int as_int;
    scalar_double as_double;
    void *generic_ptr;
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_holder {
    struct base_struct *direct_ptr;          /* Simple pointer */
    struct opaque *opaque_ptr;               /* Pointer to undefined type */
    struct array_container **double_ptr;     /* Pointer to pointer */
    union data_union *union_ptr;             /* Pointer to union */
};

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    char *user_string;
    void (*user_callback)(void);
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *chain;
    union data_union value;
    struct base_struct *base_link;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char *constant_string;
    char *dynamic_string;
    const char *array_of_strings[3];
};

/* Complete the previously undefined type */
struct GTY(()) opaque {
    struct base_struct *revealed;
    union data_union secret;
};

/* Complex nested type hierarchy */
struct GTY(()) nested_inner {
    struct base_struct base;
    union data_union data;
    struct opaque *opaque_link;
};

struct GTY(()) nested_middle {
    struct nested_inner inner;
    struct pointer_holder *ptr_holder;
    callback_t callbacks[3];
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;
    struct nested_middle middle;
    
    /* TYPE_UNION member */
    union data_union choice;
    
    /* TYPE_POINTER members */
    struct pointer_holder *ptr;
    struct user_defined_struct *user_struct;
    struct lang_tree_node *lang_node;
    
    /* TYPE_ARRAY members */
    struct nested_inner inner_array[4];
    struct base_struct *ptr_array[8];
    
    /* TYPE_STRING members */
    struct string_container strings;
    
    /* TYPE_CALLBACK member */
    complex_callback_t advanced_callback;
    
    /* TYPE_SCALAR members */
    scalar_int counter;
    scalar_double precision;
    
    /* Self-referential pointer */
    struct top_level *next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structure */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
