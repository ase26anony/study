#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(const char *, int);

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
    scalar_int as_int;
    scalar_double as_double;
    struct base_struct *as_struct;
    char *as_string;  /* TYPE_STRING will be triggered through this */
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_handle;
};

/* Nested struct with complex type composition */
struct GTY(()) nested_struct {
    struct base_struct *base;      /* TYPE_POINTER to TYPE_STRUCT */
    union data_union data;         /* TYPE_UNION */
    struct array_container arrays; /* TYPE_STRUCT containing TYPE_ARRAY */
    struct opaque *future;         /* TYPE_POINTER to TYPE_UNDEFINED */
    callback_t handlers[3];        /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *left;
    struct lang_tree_node *right;
    union data_union *attributes;
};

/* Another struct with string member for TYPE_STRING */
struct GTY(()) string_container {
    const char *name;      /* TYPE_STRING */
    char *buffer;          /* TYPE_STRING */
    char static_str[256];  /* TYPE_ARRAY of char */
};

/* Top-level complex struct that includes everything */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;
    struct nested_struct nested;
    struct array_container arrays;
    
    /* TYPE_UNION member */
    union data_union choice;
    
    /* TYPE_POINTER members */
    struct opaque *unknown;           /* To TYPE_UNDEFINED */
    struct user_defined_struct *user; /* To TYPE_USER_STRUCT */
    struct lang_tree_node *lang;      /* To TYPE_LANG_STRUCT */
    struct string_container *strings; /* To another TYPE_STRUCT */
    
    /* TYPE_ARRAY members */
    struct base_struct *ptr_array[8];  /* Array of pointers */
    callback_t callbacks[4];           /* Array of callbacks */
    
    /* TYPE_CALLBACK member */
    another_callback_t processor;
    
    /* TYPE_SCALAR members */
    scalar_int count;
    scalar_double total;
    
    /* TYPE_STRING member */
    const char *description;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Complete the previously undefined type */
struct GTY(()) opaque {
    int hidden_data;
    struct top_level *link_back;
};

#endif /* TEST_GTY_H */
