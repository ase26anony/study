#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* Union type */
union GTY(()) data_union {
    int GTY((skip)) int_val;
    float GTY((skip)) float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure (recursive) */
struct GTY(()) linked_node {
    int GTY((skip)) value;
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* User-defined type via typedef */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Complex nested structure */
struct GTY(()) container {
    base_struct_t GTY((skip)) item;
    data_union_t GTY((skip)) data;
    struct_ptr_array GTY((skip)) ptrs;
    callback_func GTY((skip)) handler;
};

/* Scalar type in container */
struct GTY(()) scalar_container {
    long GTY((skip)) long_val;
    unsigned long long GTY((skip)) ull_val;
    double GTY((skip)) double_val;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_type {
    int GTY((skip)) x;
    char GTY((skip)) y;
};
#else
struct conditional_type {
    int x;
    char y;
};
#endif

/* Multiple GTY options */
struct GTY(()) options_test {
    int GTY((skip, tag("skip_tag"))) skipped_field;
    char *GTY((length("len_field"))) variable_array;
    int GTY((desc("desc_field"))) descriptive;
    int len_field;
    int desc_field;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_child;

/* Simulating lang_struct pattern (GCC internal style) */
struct GTY((tag("lang_struct"))) lang_type {
    int GTY((skip)) lang_specific;
    struct tree_node *GTY((skip)) tree;
};

/* Mutual recursion structures */
struct GTY(()) tree_node {
    int GTY((skip)) type;
    struct tree_child *GTY((skip)) first_child;
    struct lang_type *GTY((skip)) lang_info;
};

struct GTY(()) tree_child {
    struct tree_node *GTY((skip)) node;
    struct tree_child *GTY((skip)) next;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) mutable_string;
};

/* Array of different types */
union GTY(()) variant_array_element {
    int GTY((skip)) i;
    float GTY((skip)) f;
    struct base_struct *GTY((skip)) s;
};

typedef union variant_array_element GTY(()) variant_array[20];

/* External declarations */
extern struct base_struct GTY(()) global_base;
extern data_union_t GTY(()) global_union;

#endif /* GTY_TEST_TYPES_H */
