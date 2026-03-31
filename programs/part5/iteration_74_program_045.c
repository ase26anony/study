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
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct * GTY((tag("0"))) base_ptr;
    scalar_int int_data;
    scalar_double double_data;
    void * GTY((tag("1"))) generic_ptr;
};

/* TYPE_ARRAY: Array types */
typedef scalar_int int_array[10];
typedef struct base_struct * GTY((length("5"))) struct_ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct * base_ptr_t;
typedef union data_union * union_ptr_t;

/* Nested struct with multiple type kinds */
struct GTY(()) nested_struct {
    /* TYPE_POINTER */
    struct base_struct * GTY((skip)) base_pointer;
    
    /* TYPE_ARRAY */
    int_array numbers;
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_CALLBACK */
    callback_t callback_func;
    
    /* TYPE_STRING (char pointers are treated as strings) */
    const char * GTY((skip)) name;
    
    /* Pointer to undefined type */
    struct opaque * GTY((skip)) opaque_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct type */
struct user_def GTY((user)) {
    int user_data;
    void *user_pointer;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking GCC's tree_node-like structure */
struct GTY(()) lang_tree_node {
    union {
        struct base_struct * GTY((tag("0"))) base;
        struct nested_struct * GTY((tag("1"))) nested;
        scalar_int GTY((tag("2"))) int_val;
    } GTY((desc("((lang_tree_node*)this)->node_type"))) u;
    
    int node_type;
    
    /* Chain of language nodes */
    struct lang_tree_node * GTY((skip)) next;
};

/* Complex top-level struct containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT */
    struct base_struct base;
    
    /* TYPE_UNION */
    union data_union main_data;
    
    /* TYPE_POINTER */
    struct nested_struct * GTY((skip)) nested_ptr;
    
    /* TYPE_ARRAY */
    struct_ptr_array ptr_array;
    
    /* TYPE_USER_STRUCT */
    struct user_def * GTY((skip)) user_struct;
    
    /* TYPE_LANG_STRUCT */
    struct lang_tree_node * GTY((skip)) lang_node;
    
    /* TYPE_CALLBACK */
    complex_callback_t lang_callback;
    
    /* TYPE_SCALAR */
    scalar_int counter;
    scalar_double precision;
    
    /* TYPE_STRING */
    const char * GTY((skip)) description;
    
    /* Pointer to undefined type (now defined) */
    struct opaque * GTY((skip)) opaque_data;
};

/* Define the previously opaque type */
struct opaque {
    int hidden_field;
    struct top_level * GTY((skip)) back_ref;
};

/* TYPE_ARRAY of complex type */
typedef struct top_level top_level_array[3];

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Another root with array type */
extern GTY(()) top_level_array global_array;

#endif /* TEST_GTY_H */
