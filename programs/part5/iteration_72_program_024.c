#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip option */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;  /* Recursive pointer */
    struct list_node *GTY((skip)) prev;
    union data_union data;
    int_array numbers;  /* Array field */
};

/* Callback (function pointer) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, struct base_struct*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    unsigned GTY((skip)) flags;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;  /* String pointer */
    char *GTY((skip)) buffer;
};

/* Nested complex type */
struct GTY(()) complex_type {
    struct list_node *GTY((skip)) node_list;
    union data_union variant;
    callback_func GTY((skip)) handler;
    struct_ptr_array refs;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    char *GTY((skip)) y;
};
#else
struct conditional_struct {
    int x;
    char *y;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) options_test {
    struct list_node *GTY((skip, tag("next"))) chain;
    int GTY((length("len"))) variable_array[1];  /* Variable length array hint */
    int len;
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_structs are often tagged with special prefixes */
struct GTY(()) lang_type {
    struct GTY((tag("0"))) lang_type *next;
    int lang_specific;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_container;

/* Complete definition for mutual recursion */
struct GTY(()) tree_node {
    struct tree_container *GTY((skip)) parent;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    int value;
};

struct GTY(()) tree_container {
    struct tree_node *GTY((skip)) root;
    int count;
};

/* Extern declarations for cross-file references */
extern struct base_struct GTY(()) global_base;
extern struct list_node *GTY((skip)) global_list;

#endif /* GTY_TEST_TYPES_H */
