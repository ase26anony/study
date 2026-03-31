/* test_gty.h - Test header for gengtype state coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(const char *, double);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    char name[32];
};

/* TYPE_ARRAY: Array type (implicit through struct member) */
struct GTY(()) array_container {
    int numbers[10];
    struct base_struct *struct_array[5];
    callback_t callbacks[3];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *gs;
    struct array_container *ga;
    scalar_int as_int;
    scalar_double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer types in nested context */
struct GTY(()) nested_struct {
    struct base_struct *GTY((tag("0"))) base_ptr;
    union data_union *GTY((skip)) union_ptr;
    struct opaque *GTY((skip)) opaque_ptr;  /* TYPE_UNDEFINED reference */
    int *int_ptr;
    callback_t callback_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct user_data {
    int custom_field;
    void *user_pointer;
};

struct GTY((user)) user_wrapper {
    struct user_data *data;
    int count;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node *GTY((tag("1"))) left;
        struct base_struct *base;
    } u;
    struct lang_tree_node *GTY((tag("2"))) right;
    long long int flags;
};

/* Another language-like structure */
struct GTY(()) lang_type {
    unsigned int align;
    struct lang_tree_node *GTY((tag("3"))) main_node;
    union data_union variant;
};

/* TYPE_STRING: String pointer (special case in gengtype) */
struct GTY(()) string_container {
    const char *GTY((length("strlen(%h.str_field)+1"))) str_field;
    char *dynamic_str;
    const char *constant_str;
};

/* Complex top-level struct with all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT nested */
    struct base_struct base;
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_POINTER to various types */
    struct nested_struct *nested;
    struct array_container *arrays;
    struct user_wrapper *user_struct;
    struct lang_tree_node *lang_node;
    struct lang_type *lang_type_ptr;
    struct string_container *strings;
    
    /* TYPE_ARRAY */
    struct nested_struct *ptr_array[8];
    union data_union union_array[4];
    callback_t callback_array[2];
    
    /* TYPE_CALLBACK */
    callback_t callback;
    another_callback_t another_callback;
    
    /* TYPE_SCALAR */
    scalar_int counter;
    scalar_double total;
    
    /* TYPE_STRING members */
    const char *description;
    char *buffer;
    
    /* Reference to undefined type */
    struct opaque *future_type;
    
    /* Self-reference for recursion */
    struct top_level *GTY((skip)) next;
};

/* Complete the previously undefined type */
struct GTY(()) opaque {
    struct top_level *GTY((tag("4"))) owner;
    int secret;
    struct opaque *GTY((skip)) chain;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional global variables to ensure processing */
extern GTY(()) struct lang_tree_node *global_lang_root;
extern GTY(()) union data_union global_union_var;
extern GTY(()) struct string_container global_strings;

#endif /* TEST_GTY_H */
