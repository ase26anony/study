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
typedef int (*another_callback)(void*, double);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    scalar_int id;
    scalar_double value;
    callback_t callback;  /* TYPE_CALLBACK member */
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int fixed_array[10];
    struct base_struct* ptr_array[5];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct* gty_ptr;
    void* raw_ptr;
    long long_data;
    double double_data;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int user_data;
    char* user_string;
    struct base_struct* link;
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_heavy {
    struct base_struct* direct_ptr;           /* Direct struct pointer */
    struct forward_declared* forward_ptr;     /* Forward declared pointer */
    struct opaque* opaque_ptr;                /* Opaque pointer */
    union data_union* union_ptr;              /* Union pointer */
    struct user_defined_struct* user_ptr;     /* User struct pointer */
    void (*func_ptr)(void);                   /* Function pointer */
    int* scalar_ptr;                          /* Scalar pointer */
    int (*array_ptr)[10];                     /* Array pointer */
};

/* Complete the forward declaration */
struct GTY(()) forward_declared {
    int tag;
    struct pointer_heavy* back_ref;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union {
        struct lang_tree_node* gty_children[2];
        void* raw_children[2];
    } GTY((desc("code"))) u;
    struct lang_tree_node* chain;
    long long int_value;
};

/* Complete the opaque type definition */
struct GTY(()) opaque {
    int secret;
    struct base_struct* revealed;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char* constant_string;
    char* dynamic_string;
    const char* fixed_string_array[3];
};

/* Nested complex type with all kinds */
struct GTY(()) nested_complex {
    struct base_struct base;
    union data_union data;
    struct array_container arrays;
    struct pointer_heavy* pointers;
    struct user_defined_struct* user_struct;
    struct lang_tree_node* lang_node;
    struct string_container strings;
    callback_t callbacks[2];
    struct nested_complex* next;  /* Self-referential pointer */
};

/* Top-level root structure containing everything */
struct GTY(()) top_level {
    int version;
    struct nested_complex* complex_data;
    struct opaque* opaque_data;
    struct forward_declared* forward_data;
    union data_union current_union;
    struct base_struct direct_struct;
    struct user_defined_struct user_data;
    struct lang_tree_node* lang_structure;
    struct string_container all_strings;
    void (*global_callback)(struct top_level*);
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root for language structure */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
