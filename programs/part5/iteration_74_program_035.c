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
    callback_t as_callback;
};

/* TYPE_ARRAY: Array type definition */
typedef scalar_int int_array[10];
typedef struct base_struct * GTY((length("5"))) struct_ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct * base_ptr;
typedef union data_union * union_ptr;

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    void * GTY((skip)) user_data;
    scalar_int user_id;
    const char * GTY((atomic)) user_name;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("%1.code"))) data;
    struct lang_tree_node * GTY((chain_next("%h.next"))) next;
    struct lang_tree_node * GTY((chain_prev("%h.prev"))) prev;
};

/* Nested struct with complex members */
struct GTY(()) nested_struct {
    struct base_struct base;           /* Embedded struct */
    union data_union data;             /* Embedded union */
    struct opaque * GTY((skip)) opaque_ptr;  /* Pointer to undefined type */
    base_ptr ptr_member;               /* Pointer to struct */
    int_array array_member;            /* Array of scalars */
    struct_ptr_array ptr_array;        /* Array of pointers */
    struct user_defined_struct * GTY((user)) user_struct;  /* User struct pointer */
    struct lang_tree_node * GTY((tag("1"))) lang_node;     /* Language struct pointer */
    complex_callback_t complex_callback;  /* Complex callback */
};

/* Top-level struct containing all types */
struct GTY(()) top_level {
    /* Scalar types */
    scalar_int counter;
    scalar_double total;
    
    /* Struct types */
    struct base_struct base_instance;
    struct nested_struct nested_instance;
    
    /* Union type */
    union data_union current_data;
    
    /* Pointer types */
    struct base_struct * GTY((reorder("base_struct_reorder"))) base_ptr;
    struct nested_struct * nested_ptr;
    union data_union * union_ptr;
    
    /* Array types */
    int_array scores;
    struct base_struct * GTY((length("3"))) ptr_array[3];
    
    /* String type (TYPE_STRING) - char* with special handling */
    const char * GTY((atomic)) name;
    char * GTY((length("strlen(%h.data)+1"))) data;
    
    /* Callback type */
    callback_t handler;
    complex_callback_t advanced_handler;
    
    /* User struct type */
    struct user_defined_struct * user_data;
    
    /* Language struct type */
    struct lang_tree_node * lang_root;
    
    /* Self-referential pointer */
    struct top_level * GTY((skip)) self_ptr;
    
    /* Pointer to undefined type */
    struct opaque * future;
};

/* TYPE_UNDEFINED: Now define the previously opaque struct */
struct GTY(()) opaque {
    struct top_level * GTY((tag("0"))) owner;
    scalar_int magic_number;
    struct opaque * GTY((chain_next("%h.next"))) next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structures */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
