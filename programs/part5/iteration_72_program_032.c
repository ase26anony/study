#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this field */
};

/* User-defined structure type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union data;
    int GTY((skip)) metadata;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) event_callback)(int event_id, void *GTY((skip)) data);

/* Structure containing callback */
struct GTY(()) event_handler {
    event_callback callback;
    int handler_id;
    void *GTY((skip)) user_data;
};

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    int length;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    unsigned int GTY((skip)) flags;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct linked_node *GTY((skip)) node_list;
    union data_union variants[3];
    int_array numbers;
    struct event_handler handler;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int data;
};
#else
struct chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int data;
};
#endif

/* Recursive type structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of different types */
union GTY(()) variant_array_element {
    int int_val;
    float float_val;
    struct base_struct *struct_ptr;
    event_callback callback;
};

typedef union variant_array_element GTY(()) variant_array[8];

/* Structure with multiple GTY options */
struct GTY((length("count"))) dynamic_array {
    int count;
    int *GTY((skip)) elements;  /* Variable length array */
};

#endif /* GTY_TEST_TYPES_H */
