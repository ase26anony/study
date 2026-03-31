#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback_t)(struct opaque*, double);

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_struct {
    int user_data;
    void* user_pointer;
};

/* TYPE_STRUCT: Basic structure type */
struct GTY(()) base_struct {
    scalar_int id;                     /* TYPE_SCALAR */
    scalar_double value;               /* TYPE_SCALAR */
    struct opaque* opaque_ptr;         /* Pointer to undefined type */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct* GTY((tag("0"))) base_ptr;
    struct user_struct* GTY((tag("1"))) user_ptr;
    scalar_int int_value;
    scalar_double double_value;
    callback_t callback;               /* TYPE_CALLBACK */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int GTY((length("10"))) fixed_array[10];      /* Fixed-size array */
    struct base_struct* GTY((length("5"))) ptr_array[5]; /* Array of pointers */
    char GTY((length("strlen(name)+1"))) *name;   /* Variable-length array */
};

/* TYPE_POINTER: Pointer types in nested structure */
struct GTY(()) nested_struct {
    struct base_struct* direct_ptr;    /* TYPE_POINTER to TYPE_STRUCT */
    union data_union* union_ptr;       /* TYPE_POINTER to TYPE_UNION */
    struct array_container* array_ptr; /* TYPE_POINTER to TYPE_ARRAY container */
    
    /* Nested anonymous union */
    union {
        int anon_int;
        double anon_double;
        struct base_struct* anon_ptr;
    } GTY((desc("1"))) anonymous_union;
    
    /* Callback member */
    another_callback_t processor;      /* TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node* GTY((tag("0"))) child;
        int GTY((tag("1"))) value;
        double GTY((tag("2"))) float_value;
    } u;
    struct lang_tree_node* chain;
    struct lang_tree_node* next;
};

/* TYPE_STRING: String type */
typedef const char* GTY((length("strlen($)"))) gty_string;

/* Complex top-level structure containing all type kinds */
struct GTY(()) top_level {
    /* Basic scalars */
    scalar_int counter;                /* TYPE_SCALAR */
    scalar_double total;               /* TYPE_SCALAR */
    
    /* String */
    gty_string name;                   /* TYPE_STRING */
    
    /* Structures */
    struct base_struct base;           /* TYPE_STRUCT */
    struct nested_struct nested;       /* TYPE_STRUCT with nested types */
    
    /* User structure */
    struct user_struct* user;          /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* Union */
    union data_union data;             /* TYPE_UNION */
    
    /* Array container */
    struct array_container arrays;     /* TYPE_STRUCT containing TYPE_ARRAY */
    
    /* Language structure */
    struct lang_tree_node* tree;       /* TYPE_POINTER to TYPE_LANG_STRUCT */
    
    /* Opaque pointer */
    struct opaque* mystery;            /* TYPE_POINTER to TYPE_UNDEFINED */
    
    /* Callbacks */
    callback_t handler;                /* TYPE_CALLBACK */
    another_callback_t validator;      /* TYPE_CALLBACK */
    
    /* Pointer array with variable length */
    struct nested_struct** GTY((length("item_count"))) item_list;
    int item_count;
    
    /* Self-referential pointer */
    struct top_level* GTY((skip)) self_ptr;
};

/* TYPE_UNDEFINED now defined (completes the forward declaration) */
struct GTY(()) opaque {
    int secret;
    struct top_level* owner;
    struct opaque* next;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional global to ensure processing */
extern GTY(()) struct lang_tree_node *global_tree;

#endif /* TEST_GTY_H */
