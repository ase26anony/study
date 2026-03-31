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
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct * GTY((tag("0"))) ptr_struct;
    scalar_int as_int;
    scalar_double as_double;
    void * GTY((tag("1"))) raw_ptr;
};

/* TYPE_ARRAY: Array type (implicit through struct member) */
#define ARRAY_SIZE 10

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    char * GTY((skip)) user_string;  /* Skip for GC */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("%1.code"))) data;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
};

/* TYPE_POINTER: Various pointer types in a container */
struct GTY(()) pointer_container {
    struct base_struct * GTY((tag("0"))) ptr_to_struct;
    union data_union * GTY((tag("1"))) ptr_to_union;
    struct opaque * GTY((tag("2"))) ptr_to_opaque;  /* TYPE_UNDEFINED pointer */
    struct lang_tree_node * GTY((tag("3"))) ptr_to_lang_struct;
    struct user_defined_struct * GTY((tag("4"))) ptr_to_user_struct;
    callback_t * GTY((tag("5"))) ptr_to_callback;
    scalar_int * GTY((tag("6"))) ptr_to_scalar;
};

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    int revealed_data;
    struct base_struct * GTY((tag("0"))) connection;
};

/* Complex nested struct with all type kinds */
struct GTY(()) top_level {
    /* TYPE_SCALAR members */
    scalar_int count;
    scalar_double total;
    
    /* TYPE_STRUCT member */
    struct base_struct base;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_ARRAY members */
    scalar_int int_array[ARRAY_SIZE];
    struct base_struct * GTY((tag("0"))) ptr_array[5];
    union data_union union_array[3];
    
    /* TYPE_POINTER members */
    struct pointer_container * GTY((tag("0"))) container;
    struct top_level * GTY((tag("1"))) self_ptr;
    
    /* TYPE_LANG_STRUCT member */
    struct lang_tree_node * GTY((tag("2"))) lang_node;
    
    /* TYPE_USER_STRUCT member */
    struct user_defined_struct * GTY((tag("3"))) user_data;
    
    /* TYPE_CALLBACK member */
    complex_callback_t notify;
    
    /* TYPE_STRING: Character array (treated as string) */
    char GTY((length("strlen(%h.name) + 1"))) * name;
    
    /* For TYPE_UNDEFINED testing during traversal */
    struct forward_declared * GTY((tag("4"))) future_ptr;
};

/* Another forward declaration for nested undefined type */
struct forward_declared;

/* TYPE_STRING: Additional string type usage */
struct GTY(()) string_container {
    const char * GTY((length("strlen(%h.text) + 1"))) text;
    char * GTY((length("strlen(%h.dynamic_text) + 1"))) dynamic_text;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with array of pointers */
extern GTY(()) struct pointer_container *global_containers[4];

#endif /* TEST_GTY_H */
