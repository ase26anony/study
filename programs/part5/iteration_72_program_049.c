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
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_matrix[10][20];

/* User-defined structure type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Function pointer/callback type - TYPE_CALLBACK */
typedef void (*GTY(()) event_callback)(int event_id, void *GTY((skip)) user_data);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    double GTY((skip)) accumulator;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Nested type structure */
struct GTY(()) complex_type {
    struct linked_node *GTY((skip)) node_list;
    union data_union GTY((skip)) variant;
    int_matrix GTY((skip)) matrix;
    event_callback GTY((skip)) callback;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_node {
    struct chained_node *GTY((skip)) next;
    struct chained_node *GTY((skip)) prev;
    int GTY((skip)) value;
};
#else
struct chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int value;
};
#endif

/* Recursive type definition */
struct GTY(()) tree_node {
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
    int GTY((skip)) key;
    void *GTY((skip)) data;
};

/* Array of pointers */
typedef struct tree_node *GTY(()) node_ptr_array[50];

/* Union containing structure */
union GTY(()) struct_union {
    struct base_struct GTY((skip)) as_struct;
    struct linked_node *GTY((skip)) as_node_ptr;
    event_callback GTY((skip)) as_callback;
};

/* Structure containing union */
struct GTY(()) union_container {
    union struct_union GTY((skip)) content;
    int GTY((skip)) type_tag;
};

/* For TYPE_LANG_STRUCT simulation - using a special naming pattern */
struct GTY(()) lang_struct_sim {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* Multiple GTY options */
struct GTY((skip, tag("TAG_VALUE"))) tagged_struct {
    int GTY((skip)) value;
    char GTY((skip)) tag[32];
};

/* External declarations */
extern struct base_struct GTY(()) global_base;
extern node_ptr_array GTY(()) global_node_array;

#endif /* GTY_TEST_TYPES_H */
