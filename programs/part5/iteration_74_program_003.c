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
    int scalar_field;                     /* TYPE_SCALAR */
    const char *string_field;             /* TYPE_STRING */
    struct opaque *opaque_ptr;            /* Pointer to TYPE_UNDEFINED */
};

/* For TYPE_UNION */
union GTY(()) variant_union {
    struct base_struct *struct_ptr;       /* TYPE_POINTER to TYPE_STRUCT */
    int int_value;                        /* TYPE_SCALAR */
    double double_value;                  /* TYPE_SCALAR */
    callback_t callback_field;            /* TYPE_CALLBACK */
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];                  /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct *ptr_array[5];     /* TYPE_ARRAY of TYPE_POINTER */
    union variant_union union_array[3];   /* TYPE_ARRAY of TYPE_UNION */
};

/* For TYPE_LANG_STRUCT - mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *left;          /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct lang_tree_node *right;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
    union variant_union value;            /* TYPE_UNION */
};

/* For TYPE_POINTER and complex nesting */
struct GTY(()) nested_struct {
    struct base_struct *base_ptr;         /* TYPE_POINTER to TYPE_STRUCT */
    union variant_union *union_ptr;       /* TYPE_POINTER to TYPE_UNION */
    struct array_container *array_ptr;    /* TYPE_POINTER to TYPE_STRUCT with TYPE_ARRAY */
    struct lang_tree_node *lang_ptr;      /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct user_struct *user_ptr;         /* TYPE_POINTER to TYPE_USER_STRUCT */
    callback_t callback_member;           /* TYPE_CALLBACK */
    complex_callback_t complex_callback;  /* TYPE_CALLBACK with parameters */
};

/* Complete the previously opaque type for TYPE_STRUCT */
struct GTY(()) opaque {
    int revealed_data;
    struct nested_struct *nested;         /* TYPE_POINTER creating circular reference */
    struct opaque *next;                  /* TYPE_POINTER to same type */
};

/* Top-level structure containing all type kinds */
struct GTY(()) top_level {
    /* Basic types */
    scalar_type scalar_member;            /* TYPE_SCALAR */
    string_type string_member;            /* TYPE_STRING */
    
    /* Aggregate types */
    struct base_struct base;              /* TYPE_STRUCT */
    union variant_union variant;          /* TYPE_UNION */
    struct array_container arrays;        /* TYPE_STRUCT with TYPE_ARRAY */
    
    /* Pointer types */
    struct nested_struct *nested_ptr;     /* TYPE_POINTER to TYPE_STRUCT */
    struct opaque **opaque_double_ptr;    /* TYPE_POINTER to TYPE_POINTER */
    
    /* Language and user types */
    struct lang_tree_node *lang_node;     /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct user_struct *user_data;        /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* Callback types */
    callback_t simple_callback;           /* TYPE_CALLBACK */
    complex_callback_t param_callback;    /* TYPE_CALLBACK */
    
    /* Array of various types */
    callback_t callback_array[4];         /* TYPE_ARRAY of TYPE_CALLBACK */
    struct base_struct *struct_ptr_array[3]; /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Self-referential pointer */
    struct top_level *self_ptr;           /* TYPE_POINTER to same type */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type for more coverage */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
