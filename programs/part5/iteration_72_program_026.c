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
    struct base_struct *struct_ptr;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    int data;
    union data_union value;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned int flags;
    double GTY((skip)) precision;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;  /* String pointer */
    char buffer[256];
};

/* Nested complex type */
struct GTY(()) complex_type {
    struct linked_node *node_list;
    union data_union current_data;
    int_array numbers;
    callback_func handler;
    struct scalar_container metadata;
};

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
    struct complex_type complex;
    struct tree_node *tree;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_node {
    int id;
    struct chained_node *next;
    struct chained_node *prev;
};
#else
struct chained_node {
    int id;
    struct chained_node *next;
    struct chained_node *prev;
};
#endif

/* Array with length specifier */
struct GTY(()) variable_array {
    int count;
    int *GTY((length("count"))) items;  /* Variable length array */
};

/* Multiple GTY options */
struct GTY((skip, tag("1"))) tagged_struct {
    int type_tag;
    void *GTY((skip)) data;
};

/* Forward declarations for cross-file references */
struct GTY(()) external_struct;
union GTY(()) external_union;

#endif /* GTY_TEST_TYPES_H */
