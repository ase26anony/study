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
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    double GTY((skip)) value;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Nested complex type */
struct GTY(()) complex_type {
    struct linked_node *GTY((skip)) node_list;
    union data_union current_data;
    int_array numbers;
    callback_func GTY((skip)) handler;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int value;
};
#else
struct chained_node {
    struct chained_node *next;
    struct chained_node *prev;
    int value;
};
#endif

/* Recursive structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Union containing structure */
union GTY(()) nested_union {
    struct {
        int x;
        int y;
    } GTY((skip)) point;
    struct tree_node *GTY((skip)) node;
    callback_func GTY((skip)) func;
};

/* Array of different types */
struct GTY(()) mixed_array_container {
    int simple_array[5];
    struct base_struct *GTY((skip)) struct_array[3];
    callback_func GTY((skip)) callback_array[2];
};

/* For TYPE_LANG_STRUCT simulation - using a special tag pattern */
struct GTY(()) lang_simulated {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

#endif /* GTY_TEST_TYPES_H */
