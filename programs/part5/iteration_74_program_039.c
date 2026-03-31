#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED: forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR: basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* For TYPE_CALLBACK: function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* For TYPE_STRUCT: basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK */
};

/* For TYPE_UNION: union type */
union GTY(()) data_union {
    struct base_struct * GTY((tag("0"))) ptr_struct;
    scalar_int as_int;
    scalar_double as_double;
    void * GTY((tag("1"))) ptr_void;
};

/* For TYPE_ARRAY: struct containing arrays */
struct GTY(()) array_container {
    scalar_int fixed_array[10];           /* fixed-size array */
    struct base_struct * GTY((length("len"))) dyn_array;  /* variable-length array */
    int len;
};

/* For TYPE_POINTER: struct with various pointers */
struct GTY(()) pointer_heavy {
    struct base_struct * GTY((skip)) direct_ptr;      /* regular pointer */
    struct opaque * GTY((skip)) opaque_ptr;           /* pointer to undefined type */
    union data_union * GTY((skip)) union_ptr;
    struct array_container * GTY((skip)) array_ptr;
    void * GTY((skip)) void_ptr;
};

/* For TYPE_USER_STRUCT: user-defined structure */
struct user_data {
    int user_field1;
    double user_field2;
    void *user_pointer;
};

/* Mark it as TYPE_USER_STRUCT with GTY((user)) */
typedef struct user_data GTY((user)) user_struct_t;

/* For TYPE_LANG_STRUCT: language-specific structure */
/* Mimicking GCC's internal lang-specific struct pattern */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("%1.code"))) u;
    struct lang_tree_node * GTY((skip)) chain;
    user_struct_t * GTY((skip)) user_data;  /* contains TYPE_USER_STRUCT */
};

/* Complete the previously opaque type for TYPE_UNDEFINED resolution */
struct GTY(()) opaque {
    struct lang_tree_node * GTY((skip)) lang_node;
    scalar_int magic_number;
};

/* For TYPE_STRING: string handling */
struct GTY(()) string_container {
    const char * GTY((skip)) constant_string;
    char * GTY((skip)) dynamic_string;
};

/* Complex nested type hierarchy to exercise all cases */
struct GTY(()) nested_complex {
    struct base_struct base;              /* TYPE_STRUCT */
    union data_union data;                /* TYPE_UNION */
    struct pointer_heavy * GTY((skip)) pointers;  /* TYPE_POINTER */
    struct array_container arrays;        /* TYPE_ARRAY */
    struct lang_tree_node * GTY((skip)) lang_struct;  /* TYPE_LANG_STRUCT */
    user_struct_t * GTY((skip)) user_struct;  /* TYPE_USER_STRUCT */
    struct string_container strings;      /* TYPE_STRING */
    callback_t callbacks[3];              /* TYPE_CALLBACK array */
    struct opaque * GTY((skip)) resolved_opaque;  /* Now defined */
};

/* Top-level structure containing everything */
struct GTY(()) top_level {
    scalar_int version;                   /* TYPE_SCALAR */
    struct nested_complex complex;        /* Nested complex type */
    struct nested_complex * GTY((skip)) self_ptr;  /* Self-referential pointer */
    struct top_level * GTY((skip)) next;  /* Linked list */
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with array of pointers */
extern GTY(()) struct nested_complex * GTY((length("global_count"))) global_array[];
extern int global_count;

#endif /* TEST_GTY_H */
