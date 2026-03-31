#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/forward declaration */
struct opaque;

/* For TYPE_SCALAR */
typedef int scalar_type;

/* For TYPE_STRING */
typedef const char *string_type;

/* For TYPE_CALLBACK */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* For TYPE_STRUCT with basic members */
struct GTY(()) base_struct {
    int id;                     /* TYPE_SCALAR */
    double value;               /* TYPE_SCALAR */
    const char *name;           /* TYPE_STRING */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int as_int;
    double as_double;
    struct base_struct *as_struct;  /* TYPE_POINTER */
    void *as_pointer;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];        /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct *ptr_array[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* For TYPE_LANG_STRUCT - mimicking GCC internal language structure */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("0"))) u;
    struct lang_tree_node *chain;
    struct lang_tree_node *next;
};

/* For nested TYPE_POINTER and complex structure */
struct GTY(()) nested_struct {
    struct base_struct *base_ptr;      /* TYPE_POINTER to TYPE_STRUCT */
    union data_union data;             /* TYPE_UNION */
    struct opaque *opaque_ptr;         /* TYPE_POINTER to TYPE_UNDEFINED */
    callback_t callback_func;          /* TYPE_CALLBACK */
    complex_callback_t complex_callback; /* TYPE_CALLBACK */
    struct user_struct *user_ptr;      /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct lang_tree_node *lang_ptr;   /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct array_container arrays;     /* TYPE_STRUCT containing TYPE_ARRAY */
};

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    struct nested_struct *nested;
    void *data;
    int magic_number;
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Basic types */
    scalar_type scalar_field;           /* TYPE_SCALAR */
    string_type string_field;           /* TYPE_STRING */
    
    /* Structure types */
    struct base_struct base;            /* TYPE_STRUCT */
    struct nested_struct nested;        /* TYPE_STRUCT */
    struct user_struct user;            /* TYPE_USER_STRUCT */
    
    /* Pointer types */
    struct opaque *opaque_ptr;          /* TYPE_POINTER to TYPE_STRUCT */
    struct top_level *self_ptr;         /* TYPE_POINTER (recursive) */
    void *generic_ptr;                  /* TYPE_POINTER */
    
    /* Union type */
    union data_union data_union_field;  /* TYPE_UNION */
    
    /* Array types */
    int int_array[20];                  /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct *struct_ptr_array[8]; /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Language structure */
    struct lang_tree_node *lang_tree;   /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Callback/function pointer */
    callback_t callback_field;          /* TYPE_CALLBACK */
    
    /* For array of unions */
    union data_union union_array[4];    /* TYPE_ARRAY of TYPE_UNION */
    
    /* Flexible array member */
    struct nested_struct *flex_array GTY((length("flex_count")));
    int flex_count;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root variables to ensure coverage */
extern GTY(()) struct lang_tree_node *global_lang_root;
extern GTY(()) union data_union *global_union_root;
extern GTY(()) struct user_struct *global_user_root;

#endif /* TEST_GTY_H */
