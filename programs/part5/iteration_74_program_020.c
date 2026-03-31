#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/opaque type declaration */
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
    int fixed_array[10];
    struct base_struct *GTY((length("len"))) variable_array;
    int len;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *GTY((tag("0"))) as_struct;
    scalar_int GTY((tag("1"))) as_int;
    scalar_double GTY((tag("2"))) as_double;
    void *GTY((tag("3"))) as_pointer;
};

/* TYPE_POINTER: Pointer types in a struct */
struct GTY(()) pointer_holder {
    struct base_struct *direct_ptr;
    struct opaque *GTY((skip)) opaque_ptr;  /* TYPE_UNDEFINED pointer */
    struct array_container *container_ptr;
    union data_union *union_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct user_data {
    int user_id;
    char *user_name;
};
typedef struct user_data GTY((user)) user_struct_t;

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("%1.code"))) data;
    struct lang_tree_node *GTY((skip)) left;
    struct lang_tree_node *GTY((skip)) right;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char *GTY((length("strlen"))) dynamic_string;
    int strlen;
    char fixed_string[50];
};

/* Complete the previously opaque TYPE_UNDEFINED */
struct GTY(()) opaque {
    int revealed_data;
    struct base_struct *link;
};

/* Complex nested type hierarchy */
struct GTY(()) nested_complex {
    struct base_struct base;
    struct array_container arrays;
    union data_union variant;
    struct pointer_holder *pointers;
    user_struct_t *user_data;  /* TYPE_USER_STRUCT pointer */
    struct lang_tree_node *lang_node;  /* TYPE_LANG_STRUCT pointer */
    struct string_container strings;
    callback_t callback_func;  /* TYPE_CALLBACK member */
    complex_callback_t complex_callback;
    
    /* Self-referential pointer */
    struct nested_complex *GTY((skip)) next;
    
    /* Array of pointers */
    struct base_struct *GTY((length("ptr_count"))) ptr_array[5];
    int ptr_count;
};

/* Top-level root structure */
struct GTY(()) top_level {
    struct nested_complex complex;
    struct opaque *opaque_instance;
    union data_union top_union;
    
    /* Multiple callback types */
    callback_t callbacks[3];
    
    /* Mixed array */
    struct GTY((length("mixed_count"))) {
        struct base_struct *base_ptr;
        union data_union data;
    } mixed_array[10];
    int mixed_count;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type combinations */
extern GTY(()) union data_union global_union;
extern GTY(()) struct lang_tree_node *global_lang_root;
extern GTY((user)) user_struct_t global_user_data;

#endif /* TEST_GTY_H */
