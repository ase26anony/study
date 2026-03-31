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
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_ARRAY: Array type */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];

/* TYPE_POINTER: Pointer types */
typedef struct base_struct *GTY(()) base_ptr;
typedef void *GTY(()) generic_ptr;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    base_ptr as_ptr;
    scalar_int as_int;
    scalar_double as_double;
    int_array as_array;
};

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_struct {
    int user_data;
    char *GTY((skip)) user_name;  /* Skip for GC */
    void (*user_func)(void);
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("%0.code"))) data;
    struct lang_tree_node *GTY(()) left;
    struct lang_tree_node *GTY(()) right;
    struct opaque *GTY(()) opaque_ref;  /* TYPE_UNDEFINED reference */
};

/* TYPE_STRING: String type */
typedef const char *GTY(()) gcc_string;

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    int secret;
    struct base_struct *GTY(()) reveal;
};

/* Complex nested struct exercising multiple type kinds */
struct GTY(()) nested_struct {
    struct base_struct base;           /* TYPE_STRUCT */
    union data_union data;             /* TYPE_UNION */
    struct lang_tree_node *GTY(()) lang_node;  /* TYPE_LANG_STRUCT pointer */
    struct user_struct *GTY(()) user_ref;      /* TYPE_USER_STRUCT pointer */
    int_array numbers;                 /* TYPE_ARRAY */
    gcc_string name;                   /* TYPE_STRING */
    callback_t handlers[3];            /* TYPE_ARRAY of TYPE_CALLBACK */
    struct nested_struct *GTY(()) next; /* TYPE_POINTER to self */
    struct opaque *GTY(()) opaque_ptr;  /* TYPE_POINTER to previously undefined */
};

/* Top-level container struct */
struct GTY(()) top_level {
    struct nested_struct main;         /* TYPE_STRUCT */
    union data_union variants[2];      /* TYPE_ARRAY of TYPE_UNION */
    struct lang_tree_node *GTY(()) tree_root;  /* TYPE_POINTER to TYPE_LANG_STRUCT */
    another_callback_t validator;      /* TYPE_CALLBACK */
    volatile int flags;                /* TYPE_SCALAR with qualifier */
    const struct user_struct *GTY(()) const_user; /* TYPE_POINTER with qualifier */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root for array testing */
extern GTY(()) struct nested_struct object_pool[4];

#endif /* TEST_GTY_H */
