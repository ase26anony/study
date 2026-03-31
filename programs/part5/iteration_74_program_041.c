#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(struct opaque *, double);

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* TYPE_STRUCT: Basic structure type */
struct GTY(()) base_struct {
    int id;
    double value;
    char name[32];
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int numbers[10];
    struct base_struct *struct_array[5];
    callback_t callbacks[3];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    struct base_struct * GTY((tag("0"))) struct_ptr;
    char * GTY((tag("1"))) string_ptr;
};

/* TYPE_POINTER: Pointer types in a struct */
struct GTY(()) pointer_struct {
    struct base_struct * GTY((skip)) direct_ptr;
    struct opaque * GTY((skip)) opaque_ptr;
    struct array_container *container_ptr;
    union data_union *union_ptr;
    struct user_struct *user_ptr;
    callback_t callback_ptr;
    int *int_ptr;
    char **string_array;
};

/* Nested structure with complex type composition */
struct GTY(()) nested_struct {
    struct pointer_struct *pointers;
    union data_union data;
    struct array_container arrays;
    struct nested_struct *next;  /* Self-referential pointer */
    struct nested_struct *prev;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node * GTY((tag("0"))) unary_operand;
        struct {
            struct lang_tree_node * GTY((tag("1"))) left;
            struct lang_tree_node * GTY((tag("1"))) right;
        } binary;
    } u;
    struct lang_tree_node *chain;
};

/* TYPE_STRING: String type handling */
struct GTY(()) string_container {
    const char * GTY((length("strlen(%h.string_field)+1"))) string_field;
    char *dynamic_string;
    const char *static_string;
};

/* Complete the previously opaque type */
struct GTY(()) opaque {
    int revealed_data;
    struct base_struct *link;
    struct opaque *next_opaque;
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Scalars */
    int counter;
    double ratio;
    
    /* Structures */
    struct base_struct base;
    struct nested_struct nested;
    struct array_container arrays;
    struct pointer_struct pointers;
    struct string_container strings;
    
    /* Unions */
    union data_union current_data;
    
    /* Language structure */
    struct lang_tree_node *lang_node;
    
    /* Opaque (now defined) */
    struct opaque *opaque_data;
    
    /* User structure */
    struct user_struct *user_data;
    
    /* Callbacks */
    callback_t notify;
    another_callback_t processor;
    
    /* Arrays of various types */
    int int_array[20];
    struct base_struct *struct_ptr_array[8];
    callback_t callback_array[4];
    
    /* Pointer to self for recursion */
    struct top_level *self;
    
    /* Multiple indirection */
    struct top_level ***triple_ptr;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with different type for more coverage */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
