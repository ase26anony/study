#ifndef TEST_GTY_H
#define TEST_GTY_H

/* For TYPE_UNDEFINED - forward declaration of opaque type */
struct opaque;

/* For TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* For TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque*, int);

/* For TYPE_STRING - string type */
typedef const char *gty_string;

/* For TYPE_ARRAY - array type definition */
typedef int int_array[10];
typedef struct base* ptr_array[5];

/* For TYPE_USER_STRUCT - user-defined structure with special handling */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* For TYPE_STRUCT - basic structure types */
struct GTY(()) base {
    int id;
    scalar_int count;
    scalar_double value;
    gty_string name;
};

struct GTY(()) nested {
    struct base *GTY((skip)) base_ptr;  /* Pointer to another struct */
    int_array numbers;                  /* Array member */
    callback_t callback;                /* Function pointer */
};

/* For TYPE_UNION - union type */
union GTY(()) data_union {
    struct base *GTY((tag("0"))) as_base;
    struct nested *GTY((tag("1"))) as_nested;
    scalar_int as_int;
    scalar_double as_double;
    callback_t as_callback;
};

/* For TYPE_POINTER - structure containing various pointers */
struct GTY(()) pointer_container {
    struct base *direct_ptr;           /* Simple pointer */
    struct nested **double_ptr;        /* Pointer to pointer */
    union data_union *union_ptr;       /* Pointer to union */
    struct opaque *opaque_ptr;         /* Pointer to undefined type */
    int *scalar_ptr;                   /* Pointer to scalar */
    callback_t *callback_ptr;          /* Pointer to callback */
};

/* For TYPE_LANG_STRUCT - language-specific structure */
/* Mimicking GCC's internal lang_struct pattern */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node *GTY((tag("0"))) child;
        scalar_int GTY((tag("1"))) value;
        gty_string GTY((tag("2"))) string;
    } GTY((desc("code"))) u;
    struct lang_tree_node *next;
};

/* For TYPE_UNDEFINED - now define the previously opaque type */
struct GTY(()) opaque {
    int hidden_data;
    struct base *associated;
    struct opaque *next;
};

/* Complex top-level structure containing all type variations */
struct GTY(()) top_level {
    /* Scalar types */
    scalar_int integer;
    scalar_double floating;
    
    /* String type */
    gty_string description;
    
    /* Array types */
    int_array fixed_array;
    struct base* GTY((length("dynamic_count"))) *dynamic_array;
    int dynamic_count;
    
    /* Structure types */
    struct base simple_struct;
    struct nested nested_struct;
    struct user_struct user_defined;
    
    /* Union type */
    union data_union data;
    
    /* Pointer types */
    struct pointer_container *container;
    struct lang_tree_node *lang_struct;
    struct opaque *now_defined;
    
    /* Callback types */
    callback_t simple_callback;
    complex_callback_t complex_callback;
    
    /* Nested pointers and arrays */
    struct nested* GTY((skip)) *pointer_array[3];
    union data_union (*union_func_ptr)(int);
    
    /* Self-referential pointer for graph traversal */
    struct top_level *next;
    struct top_level *prev;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root variables to ensure all types are visited */
extern GTY(()) struct lang_tree_node *lang_root;
extern GTY(()) struct user_struct *user_root;
extern GTY(()) union data_union *union_root;

#endif /* TEST_GTY_H */
