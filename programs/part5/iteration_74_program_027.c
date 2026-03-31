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

/* TYPE_STRUCT with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK */
};

/* TYPE_ARRAY: Array type */
typedef int int_array[10];
typedef struct base_struct* ptr_array[5];

/* TYPE_POINTER: Pointer types */
typedef struct base_struct *base_ptr;
typedef struct opaque *opaque_ptr;

/* TYPE_UNION with GTY marker */
union GTY(()) data_union {
    base_ptr as_ptr;
    scalar_int as_int;
    scalar_double as_double;
    int_array as_array;
};

/* TYPE_USER_STRUCT: User-defined structure with special marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
    callback_t user_callback;
};

/* Nested struct with complex members */
struct GTY(()) nested_struct {
    struct base_struct *parent;      /* TYPE_POINTER to TYPE_STRUCT */
    union data_union data;           /* TYPE_UNION */
    int_array numbers;               /* TYPE_ARRAY */
    struct opaque *unknown;          /* TYPE_POINTER to TYPE_UNDEFINED */
    struct nested_struct *next;      /* TYPE_POINTER (linked list) */
    struct nested_struct *children[3]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_LANG_STRUCT: Mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union data_union lang_data;
    struct lang_tree_node *lang_chain;
    struct base_struct *associated_struct;
};

/* TYPE_STRING: String type (char arrays/pointers are often treated specially) */
typedef const char *gty_string;

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    int revealed;
    struct nested_struct *link;
    gty_string name;
};

/* Top-level complex structure containing all types */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;             /* TYPE_STRUCT */
    struct nested_struct nested;         /* TYPE_STRUCT */
    
    /* TYPE_POINTER members */
    struct opaque *opaque_ptr;           /* TYPE_POINTER to TYPE_STRUCT */
    struct user_defined_struct *user;    /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct lang_tree_node *lang_node;    /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* TYPE_UNION member */
    union data_union choice;             /* TYPE_UNION */
    
    /* TYPE_ARRAY members */
    int_array direct_array;              /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct *ptr_array[4];    /* TYPE_ARRAY of TYPE_POINTER */
    
    /* TYPE_CALLBACK member */
    another_callback_t advanced_callback; /* TYPE_CALLBACK */
    
    /* TYPE_STRING member */
    gty_string description;              /* TYPE_STRING */
    
    /* Self-referential pointer */
    struct top_level *self;              /* TYPE_POINTER */
    
    /* Array of unions */
    union data_union union_array[2];     /* TYPE_ARRAY of TYPE_UNION */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type */
extern GTY(()) struct lang_tree_node *lang_root;

#endif /* TEST_GTY_H */
