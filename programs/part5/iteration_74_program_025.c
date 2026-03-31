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
    struct base_struct *direct_ptr;      /* Pointer to struct */
    struct opaque *opaque_ptr;           /* Pointer to undefined type */
    union data_union *union_ptr;         /* Pointer to union */
    struct pointer_holder *self_ptr;     /* Self-referential pointer */
    void *void_ptr;                      /* Generic pointer */
    callback_t callback_field;           /* Function pointer */
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct user_defined GTY((user)) {
    int user_data;
    char *user_string;
    struct base_struct *nested;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *chain;
    union data_union *lang_data;
    struct user_defined *user_info;  /* TYPE_USER_STRUCT pointer */
};

/* TYPE_STRING: String type handling */
struct GTY(()) string_container {
    const char *constant_string;
    char *dynamic_string;
    const char *array_of_strings[3];
};

/* Complex nested type hierarchy */
struct GTY(()) nested_complex {
    struct base_struct base;
    struct array_container arrays;
    union data_union data;
    struct pointer_holder *pholder;
    struct lang_tree_node *lang_node;
    struct string_container strings;
    complex_callback_t lang_callback;
    struct nested_complex *next;  /* Linked list */
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Complete the previously undefined type */
    struct opaque {
        int defined_now;
        struct top_level *parent;
    } *now_defined;
    
    struct base_struct basic;
    struct array_container container;
    union data_union union_data;
    struct pointer_holder pointers;
    struct user_defined *user_struct;  /* TYPE_USER_STRUCT */
    struct lang_tree_node lang_struct; /* TYPE_LANG_STRUCT */
    struct string_container str_data;
    struct nested_complex complex_nested;
    
    /* Array of various types */
    struct base_struct *ptr_array[8];
    union data_union union_array[4];
    callback_t callback_array[2];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type for more coverage */
extern GTY(()) struct nested_complex *secondary_root;

#endif /* TEST_GTY_H */
