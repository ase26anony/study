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
    struct base_struct * GTY((tag("0"))) ptr_base;
    scalar_int GTY((tag("1"))) int_data;
    scalar_double GTY((tag("2"))) double_data;
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    scalar_int numbers[10];
    struct base_struct * GTY((length("5"))) ptr_array[5];
};

/* TYPE_POINTER: Pointer types in a struct */
struct GTY(()) pointer_struct {
    struct base_struct * GTY((skip)) direct_ptr;
    struct opaque * GTY((skip)) opaque_ptr;
    union data_union * GTY((skip)) union_ptr;
    struct array_container * GTY((skip)) array_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct user_data {
    int user_id;
    void *user_ptr;
};
typedef struct user_data GTY((user)) user_struct_t;

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
    union data_union GTY((skip)) u;
};

/* Nested struct with all type combinations */
struct GTY(()) nested_struct {
    /* TYPE_SCALAR */
    scalar_int depth;
    
    /* TYPE_POINTER */
    struct pointer_struct * GTY((skip)) ptr_field;
    
    /* TYPE_ARRAY */
    union data_union GTY((skip)) union_array[3];
    
    /* TYPE_CALLBACK */
    callback_t handler;
    
    /* TYPE_STRING (char pointers are treated as strings) */
    const char * GTY((skip)) name;
    
    /* Reference to user struct */
    user_struct_t * GTY((skip)) user_data;
    
    /* Language structure */
    struct lang_tree_node * GTY((skip)) lang_node;
};

/* TYPE_STRUCT: Top-level complex structure */
struct GTY(()) top_level {
    /* Basic scalars */
    scalar_int id;
    scalar_double score;
    
    /* String type */
    const char * GTY((skip)) description;
    
    /* Nested struct */
    struct nested_struct GTY((skip)) nested;
    
    /* Pointer to union */
    union data_union * GTY((skip)) data_ptr;
    
    /* Array of pointers */
    struct base_struct * GTY((length("8"))) ptr_list[8];
    
    /* Callback function */
    complex_callback_t notify;
    
    /* Opaque pointer (TYPE_UNDEFINED when first seen) */
    struct opaque * GTY((skip)) hidden_data;
    
    /* Self-referential pointer */
    struct top_level * GTY((skip)) next;
    
    /* User struct */
    user_struct_t GTY((skip)) user_info;
    
    /* Language structure */
    struct lang_tree_node GTY((skip)) lang_structure;
};

/* TYPE_UNDEFINED: Now define the previously opaque struct */
struct GTY(()) opaque {
    int secret;
    struct top_level * GTY((skip)) owner;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root with array type */
extern GTY(()) struct top_level * GTY((length("4"))) global_array[4];

#endif /* TEST_GTY_H */
