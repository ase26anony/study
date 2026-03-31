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

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *struct_ptr;
    scalar_int int_val;
    scalar_double double_val;
    void *generic_ptr;
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    scalar_int numbers[10];
    struct base_struct *struct_array[5];
    union data_union union_array[3];
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_heavy {
    struct base_struct *base_ptr;           /* Pointer to struct */
    struct opaque *opaque_ptr;              /* Pointer to undefined type */
    union data_union *union_ptr;            /* Pointer to union */
    struct array_container *array_ptr;      /* Pointer to array container */
    void **void_ptr_ptr;                    /* Pointer to pointer */
    callback_t *callback_ptr;               /* Pointer to callback */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct user_data {
    int user_id;
    void *user_data;
};
typedef struct GTY((user)) user_data user_data_t;

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *left;
    struct lang_tree_node *right;
    union data_union *lang_data;
    another_callback_t lang_callback;
};

/* TYPE_STRUCT: Complex nested type hierarchy */
struct GTY(()) nested_struct {
    struct base_struct base;
    union data_union data;
    struct array_container arrays;
    struct pointer_heavy *pointers;
    user_data_t user_struct;               /* TYPE_USER_STRUCT member */
    struct lang_tree_node *lang_node;      /* TYPE_LANG_STRUCT pointer */
    
    /* Complete the opaque type definition */
    struct opaque {
        int revealed;
        struct nested_struct *link;
    } *opaque_instance;
    
    /* Mixed array types */
    scalar_int mixed_array[20];
    struct base_struct *ptr_array[8];
    
    /* Callback array */
    callback_t callbacks[4];
};

/* TYPE_UNION: Another union with complex members */
union GTY(()) complex_union {
    struct nested_struct nested;
    struct pointer_heavy *ptr_heavy;
    struct lang_tree_node lang_node;
    user_data_t user_data;
    scalar_int int_array[5];
    struct base_struct *struct_list[3];
};

/* Top-level struct containing everything */
struct GTY(()) top_level {
    struct nested_struct main_nested;
    union complex_union alt_union;
    struct base_struct *base_list[10];
    union data_union union_field;
    struct opaque *opaque_field;           /* Still undefined here */
    struct lang_tree_node *lang_tree_root;
    user_data_t user_data_field;
    
    /* Self-referential pointer */
    struct top_level *next;
    
    /* Array of pointers to different types */
    void *generic_pointers[6];
    
    /* Final callback */
    callback_t final_callback;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Complete the opaque type definition for TYPE_UNDEFINED resolution */
struct opaque {
    int finally_defined;
    struct top_level *top_link;
    union complex_union *union_link;
};

#endif /* TEST_GTY_H */
