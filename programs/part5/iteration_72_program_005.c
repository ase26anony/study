#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* User-defined structure type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int GTY((skip)) int_val;
    float GTY((skip)) float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef struct linked_node *GTY(()) node_array[100];

/* Multi-dimensional array */
typedef int GTY(()) matrix_t[10][10];

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) event_callback)(int event_id, void *GTY((skip)) data);

/* Structure containing callback */
struct GTY(()) event_handler {
    event_callback GTY((skip)) callback;
    void *GTY((skip)) user_data;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    unsigned long GTY((skip)) flags;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) union_field;
    struct linked_node *GTY((skip)) node_list;
    node_array GTY((skip)) nodes;
    matrix_t GTY((skip)) matrix;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int data;
};
#else
struct GTY(()) chained_node {
    struct chained_node *GTY((skip)) next;
    struct chained_node *GTY((skip)) prev;
    int GTY((skip)) data;
};
#endif

/* Recursive type structure */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of different pointer types */
struct GTY(()) pointer_collection {
    void *GTY((skip)) void_ptr;
    int *GTY((skip)) int_ptr;
    struct base_struct **GTY((skip)) struct_ptr_ptr;
    event_callback *GTY((skip)) callback_array[5];
};

/* Union containing structure, structure containing union */
struct GTY(()) struct_with_union {
    union {
        int GTY((skip)) x;
        float GTY((skip)) y;
    } GTY((skip)) coord;
    struct {
        int GTY((skip)) type;
        union data_union GTY((skip)) data;
    } GTY((skip)) info;
};

/* For simulating LANG_STRUCT - using special naming pattern */
struct GTY(()) lang_tree_node {
    enum tree_code {
        ERROR_MARK,
        IDENTIFIER_NODE,
        TREE_LIST
    } GTY((skip)) code;
    union lang_tree_value {
        long GTY((skip)) int_val;
        double GTY((skip)) real_val;
        struct lang_tree_node *GTY((skip)) node_ptr;
    } GTY((skip)) value;
    struct lang_tree_node *GTY((skip)) lang_specific;
};

/* Forward declarations for cross-file references */
struct GTY(()) forward_declared;
typedef struct forward_declared GTY(()) forward_declared_t;

/* Template for array with length specifier (if supported) */
struct GTY(()) variable_array {
    int GTY((skip)) length;
    int *GTY((length("length"))) data;
};

#endif /* GTY_TYPES_H */
