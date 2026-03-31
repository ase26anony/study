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

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int data[10];
    struct base_struct *GTY((skip)) ptr_array[5];
};

/* TYPE_UNION: Union type */
union GTY(()) variant_data {
    struct base_struct *GTY((tag("0"))) as_struct;
    scalar_int *GTY((tag("1"))) as_int_ptr;
    scalar_double as_double;
};

/* TYPE_POINTER: Pointer types in a struct */
struct GTY(()) pointer_holder {
    struct base_struct *GTY((skip)) direct_ptr;
    struct opaque *GTY((skip)) opaque_ptr;
    union variant_data *GTY((skip)) union_ptr;
    struct array_container *GTY((skip)) array_ptr;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_defined_struct {
    void *GTY((skip)) user_data;
    int user_tag;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    union variant_data GTY((desc("%0.code"))) data;
    struct lang_tree_node *GTY((skip)) left;
    struct lang_tree_node *GTY((skip)) right;
};

/* TYPE_STRING: String pointer */
struct GTY(()) string_container {
    const char *GTY((length("strlen(%h.str) + 1"))) str;
    char *GTY((skip)) mutable_str;
};

/* Complex nested structure to exercise multiple type kinds */
struct GTY(()) nested_complex {
    /* TYPE_STRUCT member */
    struct base_struct base;
    
    /* TYPE_UNION member */
    union variant_data variant;
    
    /* TYPE_ARRAY member */
    struct pointer_holder *GTY((skip)) ptr_list[8];
    
    /* TYPE_POINTER members */
    struct user_defined_struct *GTY((skip)) user_struct_ptr;
    struct lang_tree_node *GTY((skip)) lang_struct_ptr;
    
    /* TYPE_CALLBACK member */
    callback_t callback;
    complex_callback_t complex_callback;
    
    /* TYPE_STRING member */
    struct string_container strings;
    
    /* Recursive pointer */
    struct nested_complex *GTY((skip)) next;
};

/* Complete the previously undefined type */
struct GTY(()) opaque {
    struct nested_complex *GTY((skip)) content;
    int magic_number;
};

/* Top-level structure containing all type kinds */
struct GTY(()) top_level {
    /* Various struct types */
    struct base_struct scalar_member;
    struct array_container array_member;
    struct pointer_holder pointer_member;
    
    /* Union type */
    union variant_data union_member;
    
    /* User struct */
    struct user_defined_struct user_member;
    
    /* Language struct */
    struct lang_tree_node lang_member;
    
    /* String container */
    struct string_container string_member;
    
    /* Nested complex structure */
    struct nested_complex complex_member;
    
    /* Opaque pointer (now defined) */
    struct opaque *GTY((skip)) opaque_ptr;
    
    /* Callback function */
    callback_t notify;
    
    /* Array of various pointers */
    void *GTY((skip)) void_ptr_array[4];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Root variable for gengtype to start processing */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structure */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
