#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(const char *, double);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int numbers[10];           /* Fixed-size array */
    struct base_struct *items[5]; /* Array of pointers */
    char *string_array[3];     /* Array of strings */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *gs_ptr;
    struct array_container *array_ptr;
    scalar_int as_int;
    scalar_double as_double;
    void *generic_ptr;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_handle;
    /* User structures may have custom marking routines */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *GTY((skip)) left;
    struct lang_tree_node *GTY((skip)) right;
    union data_union GTY((desc("1"))) u;
    /* Language structures often have discriminators */
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_heavy {
    struct base_struct *direct_ptr;           /* Simple pointer */
    struct opaque *opaque_ptr;                /* Pointer to undefined type */
    struct user_defined_struct *user_ptr;     /* Pointer to user struct */
    struct lang_tree_node *lang_ptr;          /* Pointer to lang struct */
    union data_union *union_ptr;              /* Pointer to union */
    struct array_container *array_ptr;        /* Pointer to array container */
    
    /* Pointer chains */
    struct pointer_heavy *next;
    struct pointer_heavy *prev;
    
    /* Function pointer */
    another_callback_t complex_callback;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char * GTY((tag("0"))) constant_string;
    char * GTY((length("strlen(%h.dynamic_string)+1"))) dynamic_string;
    unsigned char *byte_string;
};

/* Nested complex type combining everything */
struct GTY(()) nested_complex {
    struct base_struct base;
    union data_union data;
    struct array_container arrays;
    struct string_container strings;
    struct pointer_heavy *pointer_chain;
    struct user_defined_struct *user_data;
    struct lang_tree_node *lang_node;
    
    /* Self-referential pointer */
    struct nested_complex *self;
    
    /* Array of unions */
    union data_union union_array[4];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Top-level structure containing all types */
struct GTY(()) top_level {
    /* Direct members of various types */
    struct base_struct base_instance;
    union data_union current_union;
    struct array_container array_instance;
    struct string_container string_instance;
    struct pointer_heavy *pointer_structure;
    struct user_defined_struct *user_instance;
    struct lang_tree_node *lang_instance;
    struct nested_complex *complex_nested;
    
    /* For TYPE_UNDEFINED handling */
    struct opaque *forward_declared;
    
    /* Callback function */
    callback_t notify;
    
    /* Scalar types */
    scalar_int counter;
    scalar_double precision;
    
    /* Pointer array */
    struct base_struct *ptr_array[8];
    
    /* Flexible array member (GCC extension) */
    struct lang_tree_node *flex_array GTY((length("flex_count")));
    int flex_count;
};

/* Root variable for gengtype to start processing */
extern GTY(()) struct top_level *global_root;

/* Complete the previously undefined type */
struct opaque {
    struct top_level *referrer;
    int magic_number;
    struct opaque *next;
};

#endif /* TEST_GTY_H */
