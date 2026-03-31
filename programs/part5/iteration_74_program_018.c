#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(struct opaque*, double);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback_func;  /* TYPE_CALLBACK member */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct* GTY((tag("0"))) base_ptr;
    scalar_int int_data;
    scalar_double double_data;
    void* GTY((tag("1"))) generic_ptr;
};

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct base_struct* ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* base_ptr_t;
typedef union data_union* union_ptr_t;

/* Nested struct with multiple type kinds */
struct GTY(()) nested_struct {
    /* TYPE_SCALAR members */
    scalar_int count;
    scalar_double total;
    
    /* TYPE_POINTER members */
    struct base_struct* GTY((skip)) base_ptr;  /* Skip for testing */
    struct opaque* GTY((null)) opaque_ptr;     /* TYPE_UNDEFINED pointer */
    
    /* TYPE_ARRAY members */
    int_array numbers;
    ptr_array pointers;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_CALLBACK member */
    another_callback_t processor;
    
    /* Self-referential pointer */
    struct nested_struct* GTY((desc("%1.count"))) next;
};

/* TYPE_USER_STRUCT: User-defined struct type */
struct user_data {
    int user_id;
    char* user_name;
    void* user_data;
};

/* Mark as TYPE_USER_STRUCT with GTY((user)) */
typedef struct user_data GTY((user)) user_data_t;

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking GCC's internal lang_type structure pattern */
struct GTY(()) lang_tree_node {
    int lang_specific;
    struct lang_tree_node* GTY((skip)) left;
    struct lang_tree_node* GTY((skip)) right;
    union data_union* GTY((null)) lang_data;
    callback_t lang_callback;
};

/* TYPE_STRING: String type (char*) */
typedef char* gty_string;

/* Top-level complex struct containing all type kinds */
struct GTY(()) top_level {
    /* TYPE_STRUCT member */
    struct base_struct base;
    
    /* TYPE_UNION member */
    union data_union variant;
    
    /* TYPE_POINTER members */
    struct nested_struct* nested;
    struct lang_tree_node* lang_node;
    user_data_t* user_struct;  /* TYPE_USER_STRUCT pointer */
    
    /* TYPE_ARRAY members */
    struct base_struct* GTY((length("%0.array_count"))) dynamic_array[0];
    int fixed_array[20];
    
    /* TYPE_SCALAR members */
    scalar_int array_count;
    scalar_double weight;
    
    /* TYPE_STRING member */
    gty_string name;
    
    /* TYPE_CALLBACK member */
    callback_t handler;
    
    /* TYPE_UNDEFINED pointer */
    struct opaque* unknown;
    
    /* For TYPE_ARRAY of pointers */
    struct lang_tree_node* GTY((length("%0.lang_count"))) lang_nodes[0];
    int lang_count;
};

/* Complete the TYPE_UNDEFINED */
struct opaque {
    struct top_level* GTY((skip)) owner;
    int secret;
};

/* TYPE_UNION with nested struct */
union GTY(()) container {
    struct top_level top;
    struct nested_struct nested;
    scalar_int as_int;
    scalar_double as_double;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Another root with different type */
extern GTY(()) union container *global_container;

#endif /* TEST_GTY_H */
