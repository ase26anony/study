#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/opaque type declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque*, int);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int data[10];
    struct base_struct* GTY((skip)) ptr_array[5];
};

/* TYPE_UNION: Union type */
union GTY(()) variant_data {
    struct base_struct* GTY((tag("0"))) as_base;
    struct array_container* GTY((tag("1"))) as_array;
    scalar_int as_int;
    scalar_double as_double;
};

/* TYPE_POINTER: Pointer types in a struct */
struct GTY(()) pointer_holder {
    struct base_struct* GTY((skip)) direct_ptr;
    struct opaque* GTY((skip)) opaque_ptr;
    union variant_data* GTY((skip)) union_ptr;
    struct pointer_holder* GTY((skip)) self_ptr;
};

/* TYPE_USER_STRUCT: User-defined structure type */
struct user_defined;
typedef struct user_defined* GTY((user)) user_handle_t;

struct GTY((user)) user_defined {
    int user_data;
    struct base_struct* GTY((skip)) linked_struct;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node* GTY((skip)) left;
    struct lang_tree_node* GTY((skip)) right;
    union variant_data GTY((skip)) value;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char* GTY((skip)) message;
    char buffer[256];
};

/* Nested complex type with all kinds of members */
struct GTY(()) complex_nested {
    /* Scalar members */
    scalar_int counter;
    scalar_double precision;
    
    /* Pointer members */
    struct base_struct* GTY((skip)) base_ptr;
    struct array_container* GTY((skip)) array_ptr;
    union variant_data* GTY((skip)) variant_ptr;
    struct pointer_holder* GTY((skip)) holder_ptr;
    
    /* Array member */
    struct lang_tree_node* GTY((skip)) nodes[8];
    
    /* Union member */
    union variant_data current_variant;
    
    /* Callback member */
    callback_t notify;
    complex_callback_t complex_notify;
    
    /* String member */
    struct string_container* GTY((skip)) text;
    
    /* User struct member */
    user_handle_t user_data;
    
    /* Opaque pointer */
    struct opaque* GTY((skip)) unknown;
};

/* Complete the previously opaque type */
struct GTY(()) opaque {
    int revealed;
    struct complex_nested* GTY((skip)) connection;
};

/* Top-level root structure containing everything */
struct GTY(()) top_level {
    struct base_struct base;
    struct array_container arrays;
    union variant_data variant;
    struct pointer_holder pointers;
    struct lang_tree_node lang_node;
    struct string_container strings;
    struct complex_nested complex;
    struct opaque now_visible;
    user_handle_t user_handle;
    
    /* Array of various types */
    callback_t callbacks[3];
    struct base_struct* GTY((skip)) ptr_list[4];
    union variant_data variants[2];
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional global with different type for more coverage */
extern GTY(()) struct complex_nested *secondary_root;

#endif /* TEST_GTY_H */
