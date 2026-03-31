#ifndef GTY_TYPES_H
#define GTY_TYPES_H

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
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct linked_node *GTY(()) node_ptr_array[5];

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Callback (function pointer) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    unsigned long GTY((skip)) uscalar_value;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) mutable_string;
};

/* Nested type structure */
struct GTY(()) complex_type {
    struct linked_node *GTY((skip)) node_list;
    union data_union GTY((skip)) variant;
    int_array GTY((skip)) numbers;
    callback_func GTY((skip)) handler;
};

/* Recursive type definition */
struct GTY(()) tree_node {
    int GTY((skip)) value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_node {
    int GTY((skip)) data;
    struct chained_node *GTY((skip)) next;
    struct chained_node *GTY((skip)) prev;
};
#else
struct chained_node {
    int data;
    struct chained_node *next;
    struct chained_node *prev;
};
#endif

/* Multiple GTY options */
struct GTY((skip, tag("1"))) tagged_struct {
    int GTY((skip)) tag;
    void *GTY((skip)) data;
};

/* Array with length specifier (if supported) */
struct GTY(()) variable_array {
    int GTY((skip)) length;
    int GTY((length("%0.length"))) items[1];
};

/* For TYPE_LANG_STRUCT simulation - using a naming convention 
   that might trigger special handling */
struct GTY(()) lang_tree_node {
    enum { NODE_TYPE_A, NODE_TYPE_B } GTY((skip)) node_type;
    void *GTY((skip)) lang_specific;
    struct lang_tree_node *GTY((skip)) children[4];
};

/* Undefined type forward declaration */
struct GTY(()) undefined_struct;

#endif /* GTY_TYPES_H */
