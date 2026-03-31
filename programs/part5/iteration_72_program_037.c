#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

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

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    int GTY((skip)) data;
    union data_union GTY((skip)) info;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef struct list_node GTY(()) list_node_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
typedef const char *GTY(()) const_string;

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) long_val;
    unsigned long GTY((skip)) ulong_val;
    double GTY((skip)) double_val;
};

/* Complex nested structure */
struct GTY(()) complex_struct {
    struct list_node *GTY((skip)) head;
    struct list_node *GTY((skip)) tail;
    int_array GTY((skip)) counts;
    struct_ptr_array GTY((skip)) items;
    callback_func GTY((skip)) handler;
    compare_func GTY((skip)) comparator;
};

/* Self-referential union */
union GTY(()) tree_node {
    struct GTY(()) {
        int GTY((skip)) type;
        union tree_node *GTY((skip)) left;
        union tree_node *GTY((skip)) right;
    } GTY((skip)) binary;
    struct GTY(()) {
        int GTY((skip)) type;
        union tree_node *GTY((skip)) children[4];
    } GTY((skip)) nary;
    int GTY((skip)) leaf_value;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) linked_item {
    struct linked_item *GTY((skip)) next;
    struct linked_item *GTY((skip)) prev;
    int GTY((skip)) value;
};
#else
struct linked_item {
    struct linked_item *next;
    struct linked_item *prev;
    int value;
};
#endif

/* Array with length specifier */
struct GTY(()) variable_array {
    int GTY((skip)) length;
    int GTY((length("%0.length"))) data[1];
};

/* Multiple GTY options */
struct GTY(()) tagged_union {
    int GTY((skip)) tag;
    union GTY((tag("tag"))) {
        int GTY((skip)) int_val;
        float GTY((skip)) float_val;
        struct base_struct *GTY((skip)) struct_ptr;
    } GTY((skip)) value;
};

/* Forward declarations with GTY */
struct GTY(()) forward_declared;
typedef struct forward_declared GTY(()) forward_declared_t;

/* Opaque pointer type */
typedef void *GTY(()) opaque_ptr;

#endif /* GTY_TEST_TYPES_H */
