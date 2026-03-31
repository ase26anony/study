#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int GTY((skip)) int_val;
    float GTY((skip)) float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) user_struct {
    int GTY((skip)) x;
    double GTY((skip)) y;
    union data_union GTY((skip)) data;
} user_struct_t;

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    void *GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Function pointer/callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) long_val;
    unsigned long GTY((skip)) ulong_val;
    size_t GTY((skip)) size_val;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) str;
    char *GTY((skip)) mutable_str;
};

/* Nested type definitions for complex type graphs */
struct GTY(()) complex_node {
    struct complex_node *GTY((skip)) children[4];
    union data_union GTY((skip)) value;
    callback_func GTY((skip)) handler;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int GTY((skip)) marked_field;
    void *GTY((skip)) marked_ptr;
};
#else
struct conditional_struct {
    int unmarked_field;
    void *unmarked_ptr;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) options_test {
    int GTY((skip, tag("tag_value"))) tagged_field;
    struct list_node *GTY((skip, length("len_field"))) nodes;
    int GTY((skip)) len_field;
};

/* Recursive type structure */
struct GTY(()) tree_node {
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
    int GTY((skip)) value;
};

/* Union containing structures */
union GTY(()) nested_union {
    struct base_struct GTY((skip)) as_struct;
    struct list_node *GTY((skip)) as_list;
    int_array GTY((skip)) as_array;
};

/* Structure containing union */
struct GTY(()) union_container {
    union nested_union GTY((skip)) data;
    int GTY((skip)) type;
};

/* Array of different GTY-marked types */
typedef union GTY(()) variant_data {
    int GTY((skip)) int_val;
    double GTY((skip)) double_val;
    struct base_struct *GTY((skip)) struct_ptr;
    callback_func GTY((skip)) callback;
} variant_data_t;

struct GTY(()) variant_array {
    variant_data_t GTY((skip)) variants[8];
    int GTY((skip)) count;
};

/* Simulating lang_struct pattern (TYPE_LANG_STRUCT) */
/* In GCC, lang_struct types often have special handling */
struct GTY((tag("lang_struct"))) lang_compatible {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declarations with GTY */
struct GTY(()) forward_declared;
typedef struct forward_declared forward_declared_t;

struct GTY(()) another_struct {
    forward_declared_t *GTY((skip)) fwd_ptr;
    int GTY((skip)) data;
};

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */
