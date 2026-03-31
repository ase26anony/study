#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR and TYPE_STRING */
typedef int my_scalar_t;
typedef const char *my_string_t;

/* For TYPE_CALLBACK - function pointer type */
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
    struct opaque *opaque_ptr;            /* Pointer to undefined type */
};

/* For TYPE_UNION */
union GTY(()) data_union {
    int as_int;
    double as_double;
    struct base_struct *as_struct;
    void *as_pointer;
};

/* For TYPE_ARRAY */
struct GTY(()) array_container {
    int fixed_array[10];                  /* Fixed-size array */
    struct base_struct *ptr_array[5];     /* Array of pointers */
    int flexible_array[];                 /* Flexible array member */
};

/* For TYPE_POINTER in various contexts */
struct GTY(()) pointer_heavy {
    struct base_struct *direct_ptr;       /* Direct pointer */
    struct base_struct **double_ptr;      /* Pointer to pointer */
    union data_union *union_ptr;          /* Pointer to union */
    struct array_container *array_ptr;    /* Pointer to array container */
};

/* For TYPE_LANG_STRUCT - mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    union GTY((desc ("%1.code"))) lang_tree_value {
        int intval;
        double realval;
        struct lang_tree_node *nodeptr;
    } GTY((tag ("0"))) value;
    struct lang_tree_node *chain;
};

/* For nested struct/union combinations */
struct GTY(()) complex_nested {
    /* Nested anonymous struct */
    struct GTY(()) {
        int nested_a;
        double nested_b;
    } inner;
    
    /* Nested anonymous union */
    union GTY(()) {
        long union_a;
        void *union_b;
    } data;
    
    /* Array of structs */
    struct base_struct struct_array[3];
    
    /* Pointer array with callback */
    callback_t callbacks[4];
};

/* Complete the previously undefined type */
struct GTY(()) opaque {
    int now_defined;
    struct base_struct *connection;
    callback_t cleanup_fn;
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Basic types */
    my_scalar_t scalar_type;              /* TYPE_SCALAR */
    my_string_t string_type;              /* TYPE_STRING */
    
    /* Aggregate types */
    struct base_struct base;              /* TYPE_STRUCT */
    union data_union data;                /* TYPE_UNION */
    struct array_container arrays;        /* TYPE_ARRAY within struct */
    struct pointer_heavy pointers;        /* TYPE_POINTER within struct */
    
    /* Special types */
    struct user_struct user;              /* TYPE_USER_STRUCT */
    struct lang_tree_node lang_node;      /* TYPE_LANG_STRUCT */
    
    /* Callback */
    callback_t callback_field;            /* TYPE_CALLBACK */
    complex_callback_t complex_callback;
    
    /* Self-reference and circular reference */
    struct top_level *self_ptr;
    struct opaque *opaque_ptr;
    
    /* Nested complex type */
    struct complex_nested nested;
    
    /* Variable length array pointer */
    int *vla_ptr GTY((length ("%h.vla_len")));
    int vla_len;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Another root with array */
extern GTY(()) struct top_level *global_roots[];

#endif /* TEST_GTY_H */
