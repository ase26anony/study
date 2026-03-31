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
};

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
    int GTY((skip)) data[10];
    struct base_struct *GTY((tag("0"))) ptr_array[5];
};

/* TYPE_UNION: Union type */
union GTY(()) variant_data {
    struct base_struct *GTY((tag("1"))) base_ptr;
    struct array_container *GTY((tag("2"))) array_ptr;
    scalar_int as_int;
    scalar_double as_double;
};

/* TYPE_POINTER: Pointer-only struct */
struct GTY(()) pointer_chain {
    struct pointer_chain *GTY((skip)) next;
    struct opaque *GTY((skip)) opaque_ref;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) user_defined {
    void *GTY((skip)) user_data;
    int user_tag;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internals) */
struct GTY(()) lang_tree_node {
    int code;
    union variant_data GTY((desc("%1.code"))) data;
    struct lang_tree_node *GTY((skip)) children[2];
};

/* TYPE_STRING: String handling */
struct GTY(()) string_container {
    const char *GTY((length("strlen(%h.str) + 1"))) str;
    char *GTY((skip)) mutable_str;
};

/* Complete the previously opaque type */
struct GTY(()) opaque {
    int revealed;
    struct base_struct *GTY((skip)) link;
};

/* Complex nested type hierarchy */
struct GTY(()) nested_inner {
    struct base_struct base;
    union variant_data variant;
    callback_t GTY((skip)) callback_func;
};

struct GTY(()) nested_outer {
    struct nested_inner inner;
    struct pointer_chain *chain;
    struct array_container arrays[3];
    struct user_defined *user;
    struct lang_tree_node *lang_node;
    struct string_container strings[2];
    complex_callback_t GTY((skip)) complex_callback;
};

/* Top-level root structure containing all types */
struct GTY(()) top_level {
    struct base_struct base;
    struct nested_outer outer;
    struct opaque *opaque_ptr;
    union variant_data main_variant;
    int scalar_field;
    const char *GTY((skip)) description;
    callback_t handlers[3];
};

/* Root variable for gengtype to trace */
extern GTY(()) struct top_level *global_root;

/* Additional root to ensure all types are referenced */
extern GTY(()) struct lang_tree_node *global_lang_root;

#endif /* TEST_GTY_H */
