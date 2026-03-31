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
    void * GTY((tag("1"))) ptr_void;
};

/* TYPE_ARRAY: Array type definition */
typedef scalar_int int_array[10];
typedef struct base_struct * GTY((length("5"))) struct_ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int user_data;
    char * GTY((skip)) user_string;  /* Skip for GC */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node * GTY((chain_next("%h.next"))) next;
    union data_union GTY((desc("%1.as_int"))) data;
};

/* Nested struct with complex members */
struct GTY(()) nested_struct {
    struct base_struct base;           /* Embedded struct */
    union data_union data;             /* Embedded union */
    struct opaque * GTY((skip)) opaque_ptr;  /* TYPE_UNDEFINED pointer */
    int_array numbers;                 /* TYPE_ARRAY */
    struct_ptr_array struct_ptrs;      /* TYPE_ARRAY of pointers */
    struct nested_struct * GTY((reorder("reorder_nested_struct"))) self_ptr; /* Self-reference */
    struct user_defined_struct * GTY((user)) user_struct; /* TYPE_USER_STRUCT pointer */
    struct lang_tree_node *lang_node;  /* TYPE_LANG_STRUCT pointer */
    complex_callback_t complex_cb;     /* TYPE_CALLBACK with struct param */
};

/* Complete the previously undefined struct */
struct opaque {
    struct nested_struct * GTY((skip)) owner;
    int hidden_data;
};

/* Top-level struct containing all type variations */
struct GTY(()) top_level {
    /* TYPE_STRUCT members */
    struct base_struct base;
    struct nested_struct nested;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_POINTER members */
    struct base_struct *ptr_to_base;
    struct nested_struct *ptr_to_nested;
    union data_union *ptr_to_union;
    struct user_defined_struct *ptr_to_user;
    struct lang_tree_node *ptr_to_lang;
    
    /* TYPE_ARRAY members */
    int direct_array[20];
    struct base_struct *array_of_ptrs[15];
    union data_union union_array[8];
    
    /* TYPE_SCALAR members */
    scalar_int count;
    scalar_double precision;
    
    /* TYPE_STRING member (char* is treated as TYPE_STRING) */
    const char * GTY((length("strlen(%h.text) + 1"))) text;
    
    /* TYPE_CALLBACK member */
    callback_t handler;
    
    /* TYPE_UNDEFINED (now defined) */
    struct opaque *opaque;
    
    /* Chain pointer for traversal */
    struct top_level * GTY((chain_next("%h.next"))) next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root with different structure */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
