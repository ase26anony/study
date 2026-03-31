/* test_gty.h - Test header for gengtype state coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/opaque type declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    char name[32];
};

/* TYPE_ARRAY: Array type (implicit through struct member) */
struct GTY(()) array_container {
    int numbers[10];
    struct base_struct *struct_array[5];
    callback_t callbacks[3];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int as_int;
    double as_double;
    void *as_pointer;
    struct base_struct *as_struct;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;
typedef callback_t *callback_ptr;

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_handle;
    /* User structs often have custom marking functions */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking GCC's tree_node-like structure */
struct GTY(()) lang_tree_node {
    int code;
    union {
        int ival;
        double dval;
        void *ptr;
        struct lang_tree_node *gt_ggc_m_rtree_node;
    } GTY((desc ("%0.code"))) u;
    struct lang_tree_node *chain;
    struct lang_tree_node *next;
};

/* Complex nested struct with all type kinds */
struct GTY(()) nested_complex {
    /* TYPE_SCALAR */
    int counter;
    double ratio;
    
    /* TYPE_POINTER */
    struct base_struct *base_ptr;
    struct opaque *opaque_ptr;  /* TYPE_UNDEFINED reference */
    struct user_defined_struct *user_ptr;
    struct lang_tree_node *lang_ptr;
    
    /* TYPE_ARRAY */
    int matrix[4][4];
    union data_union union_array[8];
    
    /* TYPE_UNION */
    union data_union current_data;
    
    /* TYPE_STRUCT (embedded) */
    struct array_container container;
    
    /* TYPE_CALLBACK */
    callback_t notify;
    another_callback process;
    
    /* Nested pointer to same type */
    struct nested_complex *self;
    
    /* Pointer chain */
    struct nested_complex *next;
    struct nested_complex *prev;
};

/* TYPE_STRUCT: Top-level struct containing everything */
struct GTY(()) top_level {
    /* Complete the opaque type definition (now TYPE_STRUCT) */
    struct GTY(()) opaque {
        int revealed;
        void *data;
        struct nested_complex *link;
    } *opaque_instance;
    
    /* Various struct types */
    struct base_struct base;
    struct array_container arrays;
    struct user_defined_struct user_struct_instance;
    struct lang_tree_node *lang_tree_root;
    
    /* Union */
    union data_union variant;
    
    /* Complex nested structure */
    struct nested_complex complex;
    
    /* Pointer array */
    struct nested_complex *ptr_list[16];
    
    /* Callback registry */
    callback_t handlers[10];
    
    /* Dynamic array (as pointer + size) */
    int *dynamic_array;
    unsigned int array_size;
    
    /* Linked list of various types */
    struct top_level *next_top;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structure */
extern GTY(()) struct lang_tree_node *global_lang_root;

/* Function prototypes that might be referenced */
void GTY((user)) register_callback(callback_t cb);
struct opaque* GTY(()) create_opaque(void);

#endif /* TEST_GTY_H */
