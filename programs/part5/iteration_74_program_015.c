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
    callback_t callback;  /* TYPE_CALLBACK */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *struct_ptr;
    scalar_int int_val;
    scalar_double double_val;
    void *generic_ptr;
};

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct base_struct *struct_ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;

/* Nested struct with multiple type kinds */
struct GTY(()) nested_struct {
    base_ptr parent;              /* TYPE_POINTER */
    union data_union data;        /* TYPE_UNION */
    int_array numbers;            /* TYPE_ARRAY */
    struct_ptr_array children;    /* TYPE_ARRAY of TYPE_POINTER */
    callback_t handlers[3];       /* TYPE_ARRAY of TYPE_CALLBACK */
    char *name;                   /* TYPE_STRING (char*) */
};

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    int user_id;
    void *user_data;
    struct nested_struct *nested;  /* TYPE_POINTER */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *left;   /* TYPE_POINTER */
    struct lang_tree_node *right;  /* TYPE_POINTER */
    union data_union attr;         /* TYPE_UNION */
    struct user_defined_struct *user_data;  /* TYPE_POINTER to TYPE_USER_STRUCT */
};

/* Complete the previously undefined type */
struct GTY(()) opaque {
    struct lang_tree_node *lang_node;  /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct user_defined_struct *user;  /* TYPE_POINTER to TYPE_USER_STRUCT */
    callback_t finalizer;              /* TYPE_CALLBACK */
};

/* Top-level complex struct containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;           /* TYPE_STRUCT */
    struct nested_struct nested;       /* TYPE_STRUCT */
    
    /* TYPE_POINTER members */
    struct opaque *opaque_ptr;         /* TYPE_POINTER to completed TYPE_UNDEFINED */
    struct lang_tree_node *lang_root;  /* TYPE_POINTER to TYPE_LANG_STRUCT */
    struct user_defined_struct *user_struct;  /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* TYPE_UNION member */
    union data_union variant;          /* TYPE_UNION */
    
    /* TYPE_ARRAY members */
    struct base_struct *struct_array[8];  /* TYPE_ARRAY of TYPE_POINTER */
    callback_t callback_array[4];         /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* TYPE_STRING members */
    const char *string_const;          /* TYPE_STRING */
    char *string_var;                  /* TYPE_STRING */
    
    /* TYPE_SCALAR members */
    scalar_int count;
    scalar_double total;
    
    /* TYPE_CALLBACK member */
    complex_callback_t notify;         /* TYPE_CALLBACK */
    
    /* Self-referential pointer */
    struct top_level *next;            /* TYPE_POINTER */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional global variables to ensure processing */
extern GTY(()) struct lang_tree_node *global_lang_node;
extern GTY(()) union data_union global_union;
extern GTY(()) struct user_defined_struct *global_user_struct;

#endif /* TEST_GTY_H */
