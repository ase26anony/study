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

/* TYPE_STRUCT with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int data[10];  /* TYPE_ARRAY */
    struct base_struct *ptr_array[5];  /* Array of pointers */
};

/* TYPE_UNION */
union GTY(()) variant_data {
    struct base_struct *base_ptr;  /* TYPE_POINTER */
    struct array_container *array_ptr;
    scalar_int as_int;
    scalar_double as_double;
};

/* TYPE_USER_STRUCT */
struct GTY((user)) user_defined_struct {
    void *user_data;
    int user_tag;
};

/* Nested struct with multiple pointer types */
struct GTY(()) nested_struct {
    struct base_struct *base;      /* TYPE_POINTER to TYPE_STRUCT */
    union variant_data *variant;   /* TYPE_POINTER to TYPE_UNION */
    struct opaque *forward_ref;    /* TYPE_POINTER to TYPE_UNDEFINED */
    struct user_defined_struct *user;  /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* TYPE_STRING */
    const char *name;
    
    /* Inline array */
    int scores[20];  /* TYPE_ARRAY */
};

/* TYPE_LANG_STRUCT: Mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *left;   /* Recursive pointer */
    struct lang_tree_node *right;  /* Recursive pointer */
    union variant_data data;
};

/* Complete the previously undefined type */
struct GTY(()) opaque {
    struct nested_struct *nested;
    struct lang_tree_node *lang_node;
    complex_callback_t lang_callback;
};

/* Top-level complex structure hitting all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;
    struct nested_struct nested;
    
    /* TYPE_UNION member */
    union variant_data variant;
    
    /* TYPE_POINTER members to various types */
    struct opaque *opaque_ptr;
    struct array_container *array_ptr;
    struct lang_tree_node *lang_ptr;
    struct user_defined_struct *user_ptr;
    
    /* TYPE_ARRAY members */
    struct base_struct *struct_array[8];
    union variant_data union_array[4];
    
    /* TYPE_SCALAR members */
    scalar_int count;
    scalar_double total;
    
    /* TYPE_STRING members */
    const char *description;
    const char *labels[3];
    
    /* TYPE_CALLBACK members */
    callback_t simple_callback;
    complex_callback_t complex_callback;
    
    /* Self-referential pointer */
    struct top_level *next;  /* TYPE_POINTER to same type */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structure */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
