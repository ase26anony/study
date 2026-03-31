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

/* TYPE_ARRAY: Array type */
typedef int int_array[10];
typedef struct base_struct struct_array[5];

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int as_int;
    scalar_double as_double;
    struct base_struct * GTY((tag("0"))) as_ptr;
    callback_t as_callback;
};

/* TYPE_POINTER: Pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;

/* TYPE_USER_STRUCT: User-defined struct with special marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
    union data_union data;
};

/* Complete the previously undefined struct */
struct GTY(()) opaque {
    struct base_struct * GTY((skip)) ref;
    int hidden_field;
};

/* Complex nested struct exercising multiple type kinds */
struct GTY(()) nested_struct {
    /* TYPE_SCALAR */
    scalar_int counter;
    
    /* TYPE_POINTER */
    struct base_struct * GTY((skip)) base_ptr;
    
    /* TYPE_ARRAY */
    int_array numbers;
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_STRING (char pointers are special) */
    const char * GTY((skip)) name;
    
    /* TYPE_CALLBACK */
    callback_t handler;
    
    /* TYPE_UNDEFINED -> now defined */
    struct opaque * GTY((skip)) opaque_ptr;
    
    /* TYPE_USER_STRUCT */
    struct user_defined_struct * GTY((skip)) user_struct;
    
    /* TYPE_LANG_STRUCT */
    struct lang_tree_node * GTY((skip)) lang_node;
    
    /* Nested array of pointers */
    struct base_struct * GTY((skip)) ptr_array[3];
    
    /* Multi-dimensional array */
    int matrix[2][2];
};

/* Top-level struct containing everything */
struct GTY(()) top_level {
    /* Various struct types */
    struct base_struct base;
    struct nested_struct nested;
    
    /* Union */
    union data_union main_data;
    
    /* Pointers to different types */
    struct opaque * GTY((skip)) opaque_ref;
    struct user_defined_struct * GTY((skip)) user_ref;
    struct lang_tree_node * GTY((skip)) lang_ref;
    
    /* Array of structs */
    struct base_struct items[4];
    
    /* Array of pointers */
    struct nested_struct * GTY((skip)) nested_ptrs[2];
    
    /* Callback function */
    complex_callback_t notify;
    
    /* String pointer */
    const char * GTY((skip)) description;
    
    /* Self-referential pointer */
    struct top_level * GTY((skip)) next;
};

/* TYPE_STRING: Special string structure */
struct GTY(()) string_wrapper {
    const char * GTY((skip)) content;
    int length;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Another root with different type for more coverage */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
