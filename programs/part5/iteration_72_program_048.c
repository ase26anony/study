#ifndef GTY_TYPES_H
#define GTY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip attribute */
};

/* GTY-marked union - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    void *GTY((skip)) ptr_val;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;  /* Recursive pointer */
    struct linked_node *GTY((skip)) prev;  /* Another pointer */
    int data;
    union data_union GTY((skip)) value;    /* Union within struct */
};

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;  /* String pointer */
    char *GTY((skip)) buffer;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    unsigned GTY((skip)) flags;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* Complex nested structure */
struct GTY(()) complex_type {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) data;
    struct linked_node *GTY((skip)) node_list;
    int_array GTY((skip)) numbers;
    callback_func GTY((skip)) handler;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    double y;
};
#else
struct conditional_struct {
    int x;
    double y;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) options_test {
    int *GTY((skip, tag("optional"))) optional_ptr;
    struct linked_node **GTY((skip)) node_matrix[3][3];
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array with length attribute (simulated) */
struct GTY(()) variable_array {
    int count;
    int *GTY((skip, length("count"))) items;  /* TYPE_ARRAY with length */
};

/* Union containing structures */
union GTY(()) struct_union {
    struct base_struct GTY((skip)) as_struct;
    struct linked_node GTY((skip)) as_node;
    struct scalar_box GTY((skip)) as_scalar;
};

#ifdef __cplusplus
}
#endif

#endif /* GTY_TYPES_H */
