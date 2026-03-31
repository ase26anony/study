#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(struct opaque *, double);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int numbers[10];           /* Fixed-size array */
    struct base_struct *items[5]; /* Array of pointers */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *ptr;
    scalar_int as_int;
    scalar_double as_double;
    void *generic_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    char *user_string;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *chain;
    union data_union *data;
    void GTY((skip)) *unmarked_ptr;  /* Skip GC marking */
};

/* TYPE_POINTER: Now define the previously opaque struct */
struct GTY(()) opaque {
    struct base_struct *base;      /* Pointer to struct */
    union data_union *union_data;  /* Pointer to union */
    struct opaque *next;           /* Self-referential pointer */
    int array[3];                  /* TYPE_ARRAY */
};

/* TYPE_STRING: String type */
typedef const char *gty_string;

/* Complex nested type hierarchy */
struct GTY(()) nested_struct {
    struct base_struct base;
    union data_union variant;
    struct opaque *opaque_ptr;
    struct nested_struct *sibling;
    struct array_container container;
    gty_string name;               /* TYPE_STRING */
    callback_t handlers[2];        /* Array of callbacks */
};

/* Top-level struct containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;
    struct nested_struct nested;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_POINTER members */
    struct opaque *opaque_ptr;
    struct user_defined_struct *user_struct_ptr;
    struct lang_tree_node *lang_node;
    
    /* TYPE_ARRAY members */
    int scalar_array[20];
    struct base_struct *struct_ptr_array[8];
    union data_union union_array[4];
    
    /* TYPE_CALLBACK member */
    another_callback_t processor;
    
    /* TYPE_STRING member */
    gty_string description;
    
    /* Nested complex types */
    struct {
        int anonymous_member;
        struct opaque *anon_opaque;
    } GTY(()) anonymous_struct;
    
    /* For TYPE_UNDEFINED testing */
    struct forward_declared *future;  /* Will be defined later */
};

/* TYPE_UNDEFINED -> TYPE_STRUCT transition */
struct GTY(()) forward_declared {
    int finally_defined;
    struct top_level *parent;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional global to ensure processing */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
