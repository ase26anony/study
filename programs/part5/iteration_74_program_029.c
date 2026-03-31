#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_USER_STRUCT - user-defined structure type */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

/* For TYPE_STRUCT - basic structure type */
struct GTY(()) base_struct {
    scalar_int id;                     /* TYPE_SCALAR */
    scalar_double value;               /* TYPE_SCALAR */
    struct base_struct * GTY((skip)) next;  /* TYPE_POINTER */
};

/* For TYPE_UNION - union type */
union GTY(()) data_union {
    scalar_int as_int;                 /* TYPE_SCALAR */
    scalar_double as_double;           /* TYPE_SCALAR */
    struct base_struct * GTY((skip)) as_struct; /* TYPE_POINTER */
    char * GTY((skip)) as_string;      /* TYPE_STRING */
};

/* For TYPE_ARRAY - structure with array member */
struct GTY(()) array_container {
    int fixed_array[10];               /* TYPE_ARRAY of TYPE_SCALAR */
    struct base_struct * GTY((skip)) ptr_array[5]; /* TYPE_ARRAY of TYPE_POINTER */
    union data_union union_array[3];   /* TYPE_ARRAY of TYPE_UNION */
};

/* For TYPE_LANG_STRUCT - language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node * GTY((skip)) child;
        scalar_int value;
        char * GTY((skip)) string;
    } GTY((desc ("code"))) u;
    struct lang_tree_node * GTY((skip)) chain;
};

/* For TYPE_STRING - string pointer type */
typedef const char * GTY((skip)) string_ptr;

/* Complete the opaque type definition (was TYPE_UNDEFINED, now TYPE_STRUCT) */
struct GTY(()) opaque {
    int revealed_data;
    struct base_struct * GTY((skip)) connection;
};

/* Complex nested structure to exercise all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;           /* TYPE_STRUCT */
    
    /* TYPE_UNION member */
    union data_union data;             /* TYPE_UNION */
    
    /* TYPE_POINTER members */
    struct opaque * GTY((skip)) opaque_ptr;     /* TYPE_POINTER to TYPE_STRUCT */
    struct array_container * GTY((skip)) container; /* TYPE_POINTER to TYPE_STRUCT */
    
    /* TYPE_ARRAY members */
    callback_t callbacks[4];           /* TYPE_ARRAY of TYPE_CALLBACK */
    string_ptr messages[3];            /* TYPE_ARRAY of TYPE_STRING */
    
    /* TYPE_CALLBACK member */
    complex_callback_t complex_cb;     /* TYPE_CALLBACK */
    
    /* TYPE_USER_STRUCT member */
    struct user_defined_struct * GTY((skip)) user_data; /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* TYPE_LANG_STRUCT member */
    struct lang_tree_node * GTY((skip)) lang_node; /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* TYPE_STRING member */
    char * GTY((skip)) dynamic_string; /* TYPE_STRING */
    
    /* Nested structure with pointer to self */
    struct GTY(()) nested {
        int level;
        struct nested * GTY((skip)) deeper;
        struct top_level * GTY((skip)) parent;
    } recursion;
    
    /* Array of unions */
    union data_union variant_array[2]; /* TYPE_ARRAY of TYPE_UNION */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type for more coverage */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
