#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
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

/* TYPE_ARRAY: Array type embedded in struct */
struct GTY(()) array_container {
    int numbers[10];
    struct base_struct *GTY((skip)) ptr_array[5];
    callback_t callbacks[3];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct *GTY((tag("0"))) base_ptr;
    struct array_container *GTY((tag("1"))) array_ptr;
    scalar_int int_value;
    scalar_double double_value;
    callback_t func_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct user_defined GTY((user)) {
    void *GTY((skip)) user_data;
    int user_tag;
    union data_union *GTY((desc("user_tag"))) variant;
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_holder {
    struct base_struct *base;                    /* Regular pointer */
    struct opaque *GTY((skip)) opaque_ptr;       /* Pointer to undefined type */
    union data_union **double_ptr;               /* Pointer to pointer */
    struct array_container *array_ref;
    struct user_defined *user_struct_ref;
    callback_t callback_member;                  /* Function pointer member */
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    enum tree_code {
        ERROR_MARK,
        IDENTIFIER_NODE,
        TREE_LIST
    } code;
    union lang_tree_value {
        long GTY((tag("0"))) int_val;
        double GTY((tag("1"))) real_val;
        struct lang_tree_node *GTY((tag("2"))) node_ptr;
    } GTY((desc("%1.code"))) value;
    struct lang_tree_node *chain;
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char *constant_string;
    char *dynamic_string;
    unsigned char byte_array[20];
};

/* Complex nested type hierarchy */
struct GTY(()) nested_struct {
    struct base_struct base;
    union data_union data;
    struct pointer_holder *holder;
    struct array_container arrays[2];
    struct string_container strings;
    struct lang_tree_node *lang_node;
    struct user_defined *user_data;
    callback_t handlers[2];
    
    /* Nested anonymous struct */
    struct {
        int anonymous_id;
        struct nested_struct *next;
    } GTY((tag("anonymous"))) link;
};

/* Top-level struct containing all types */
struct GTY(()) top_level {
    struct base_struct base_member;
    union data_union union_member;
    struct pointer_holder *pointer_member;
    struct array_container array_member;
    struct string_container string_member;
    struct lang_tree_node *lang_member;
    struct user_defined *user_member;
    struct nested_struct nested;
    
    /* Complete the opaque type definition */
    struct opaque {
        int defined_now;
        struct top_level *back_ref;
    } *opaque_ptr;
    
    /* Callback function pointer */
    callback_t notify;
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Pointer to array */
    int (*array_pointer)[10];
};

/* TYPE_UNDEFINED now becomes defined */
struct opaque {
    int finally_defined;
    struct top_level *top;
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional global with various pointer types */
extern GTY(()) union data_union *global_union_array[4];

#endif /* TEST_GTY_H */
