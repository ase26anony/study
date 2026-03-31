#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete struct declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_t;
typedef double double_scalar_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    int id;
    scalar_t value;
};

/* TYPE_UNION: Union with GTY marker */
union GTY(()) data_union {
    int as_int;
    double as_double;
    struct base_struct *GTY((tag("0"))) as_ptr;
};

/* TYPE_ARRAY: Array within a struct */
struct GTY(()) array_container {
    int numbers[10];
    struct base_struct *GTY((skip)) ptr_array[5];
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_heavy {
    struct base_struct *GTY((skip)) direct_ptr;
    struct opaque *GTY((skip)) opaque_ptr; /* TYPE_UNDEFINED pointer */
    union data_union *GTY((skip)) union_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined {
    int user_id;
    char *GTY((skip)) name;
};

/* TYPE_LANG_STRUCT: Mimic GCC language-specific structure */
struct GTY(()) lang_tree_node {
    int code;
    struct lang_tree_node *GTY((skip)) left;
    struct lang_tree_node *GTY((skip)) right;
    union data_union GTY((skip)) data;
};

/* TYPE_STRING: String pointer (char *) */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Complex nested top-level struct */
struct GTY(()) top_level {
    /* TYPE_STRUCT */
    struct base_struct base;
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_ARRAY */
    struct array_container arrays;
    
    /* TYPE_POINTER */
    struct pointer_heavy *GTY((skip)) pointers;
    
    /* TYPE_USER_STRUCT */
    struct user_defined *GTY((skip)) user;
    
    /* TYPE_LANG_STRUCT */
    struct lang_tree_node *GTY((skip)) lang_node;
    
    /* TYPE_STRING */
    struct string_holder strings;
    
    /* TYPE_CALLBACK */
    callback_t callback;
    
    /* TYPE_UNDEFINED (opaque) */
    struct opaque *GTY((skip)) opaque_field;
    
    /* Nested array of pointers */
    struct base_struct *GTY((skip)) ptr_matrix[3][3];
    
    /* Scalar members */
    scalar_t scalar;
    double_scalar_t double_scalar;
};

/* TYPE_UNDEFINED now defined (completes the incomplete type) */
struct opaque {
    struct top_level *GTY((skip)) back_ref;
    int secret;
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

#endif /* TEST_GTY_H */
