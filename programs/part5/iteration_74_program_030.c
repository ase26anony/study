#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - incomplete/forward declaration */
struct opaque;

/* For TYPE_SCALAR and basic types */
typedef int my_scalar_t;
typedef double my_double_t;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_STRUCT with GTY marker */
struct GTY(()) base_struct {
    my_scalar_t scalar_field;          /* TYPE_SCALAR */
    my_double_t double_field;          /* TYPE_SCALAR */
    char * GTY((skip)) name;           /* TYPE_STRING (skip marker for variety) */
};

/* For TYPE_UNION */
union GTY(()) my_union {
    struct base_struct * GTY((tag("0"))) ptr_to_base;  /* TYPE_POINTER */
    my_scalar_t scalar_val;                            /* TYPE_SCALAR */
    void * GTY((tag("1"))) generic_ptr;                /* TYPE_POINTER */
};

/* For TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct * GTY(()) ptr_array[5];

/* For TYPE_USER_STRUCT */
struct GTY((user)) user_defined_struct {
    /* User-defined structures require manual marking routines */
    void *data;
    int size;
};

/* For TYPE_LANG_STRUCT - mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union my_union * GTY((tag("0"))) lang_specific;
    struct lang_tree_node * GTY((chain_next("%h.next"))) next;
};

/* Complete the previously opaque type for TYPE_STRUCT */
struct GTY(()) opaque {
    int revealed;
    struct base_struct * GTY((null)) base_ptr;  /* TYPE_POINTER */
};

/* Complex nested structure to exercise multiple type kinds */
struct GTY(()) nested_struct {
    struct base_struct base;                    /* TYPE_STRUCT */
    union my_union choice;                      /* TYPE_UNION */
    struct opaque * GTY((reorder)) opaque_ptr;  /* TYPE_POINTER */
    int_array numbers;                          /* TYPE_ARRAY */
    ptr_array pointers;                         /* TYPE_ARRAY of TYPE_POINTER */
    callback_t callback_func;                   /* TYPE_CALLBACK */
    complex_callback_t complex_callback;        /* TYPE_CALLBACK */
    struct user_defined_struct * GTY((user)) user_struct; /* TYPE_USER_STRUCT */
    struct lang_tree_node * GTY((desc("tree_node"))) lang_node; /* TYPE_LANG_STRUCT */
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Direct struct member */
    struct base_struct direct;                  /* TYPE_STRUCT */
    
    /* Pointer members */
    struct nested_struct * GTY((maybe_undef)) nested_ptr; /* TYPE_POINTER */
    union my_union * GTY((skip)) union_ptr;     /* TYPE_POINTER */
    
    /* Array members */
    struct nested_struct GTY(()) struct_array[3]; /* TYPE_ARRAY of TYPE_STRUCT */
    callback_t GTY(()) callback_array[2];       /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* String type */
    const char * GTY((length("strlen(%h.str)"))) str; /* TYPE_STRING */
    
    /* Union member */
    union my_union data_union;                  /* TYPE_UNION */
    
    /* Scalar members */
    my_scalar_t count;                          /* TYPE_SCALAR */
    my_double_t value;                          /* TYPE_SCALAR */
    
    /* Callback member */
    callback_t handler;                         /* TYPE_CALLBACK */
    
    /* For undefined type reference */
    struct undefined_type * GTY((maybe_undef)) undefined_ptr; /* TYPE_UNDEFINED */
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with array */
extern GTY(()) struct nested_struct *global_array_root[4];

#endif /* TEST_GTY_H */
