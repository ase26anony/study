#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int, double);
typedef void (*lang_callback_t)(struct opaque*);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int numbers[10];           /* Fixed-size array */
    struct base_struct* items[5]; /* Array of pointers */
    scalar_double matrix[3][3];  /* Multi-dimensional array */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int as_int;
    scalar_double as_double;
    struct base_struct* as_struct;
    callback_t as_callback;
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_holder {
    struct base_struct* direct_ptr;           /* Pointer to struct */
    struct opaque* opaque_ptr;                /* Pointer to undefined type */
    union data_union* union_ptr;              /* Pointer to union */
    struct array_container* array_ptr;        /* Pointer to array container */
    struct pointer_holder* self_ptr;          /* Self-referential pointer */
    struct pointer_holder** double_ptr;       /* Pointer to pointer */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    void* user_data;
    int user_tag;
    struct base_struct* linked_item;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union value;
    struct lang_tree_node* left;
    struct lang_tree_node* right;
    struct lang_tree_node* parent;
    lang_callback_t lang_handler;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char* static_string;    /* Pointer to constant string */
    char* dynamic_string;         /* Pointer to mutable string */
    const char* array_of_strings[3]; /* Array of string pointers */
};

/* Nested complex type combining everything */
struct GTY(()) nested_complex {
    struct base_struct base;
    union data_union data;
    struct array_container arrays;
    struct pointer_holder* pointers;
    struct user_defined_struct* user_struct;
    struct lang_tree_node* lang_node;
    struct string_container strings;
    struct nested_complex* next;  /* Linked list */
};

/* Top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Scalar types */
    scalar_int counter;
    scalar_double total;
    
    /* Struct types */
    struct base_struct item;
    struct array_container collection;
    
    /* Union type */
    union data_union variant;
    
    /* Pointer types */
    struct pointer_holder* ptr_holder;
    struct opaque* unknown;
    
    /* Array types */
    struct nested_complex complex_array[2];
    callback_t callbacks[4];
    
    /* User struct */
    struct user_defined_struct user_data;
    
    /* Language struct */
    struct lang_tree_node* syntax_tree;
    
    /* String types */
    struct string_container text_data;
    
    /* Callback function */
    callback_t notify;
    
    /* Self-reference for cycles */
    struct top_level* partner;
};

/* TYPE_UNDEFINED: Now define the previously opaque struct */
struct GTY(()) opaque {
    int magic;
    struct top_level* connection;
    struct opaque* next;
};

/* Root variable for gengtype to start processing */
extern GTY(()) struct top_level *global_root;

/* Additional global variables to ensure processing */
extern GTY(()) struct lang_tree_node *global_tree;
extern GTY(()) union data_union global_union;
extern GTY(()) struct user_defined_struct *global_user_struct;

#endif /* TEST_GTY_H */
