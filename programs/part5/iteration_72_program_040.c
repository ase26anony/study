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
typedef struct base_struct GTY(()) struct_array[10];
typedef int GTY(()) int_matrix[5][5];

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) long_val;
    unsigned GTY((skip)) flags;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct linked_node *GTY((skip)) node_list;
    union data_union GTY((skip)) variant;
    struct struct_array GTY((skip)) array_field;
    callback_func GTY((skip)) handler;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_struct {
    struct chained_struct *next;
    struct chained_struct *prev;
    int data;
};
#else
struct GTY(()) chained_struct {
    struct chained_struct *GTY((skip)) next;
    struct chained_struct *GTY((skip)) prev;
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

/* Union containing structures */
union GTY(()) variant_container {
    struct base_struct GTY((skip)) as_struct;
    struct linked_node *GTY((skip)) as_list;
    struct tree_node *GTY((skip)) as_tree;
    callback_func GTY((skip)) as_func;
};

/* Array of pointers to different types */
typedef struct base_struct* GTY(()) struct_ptr_array[20];
typedef union data_union* GTY(()) union_ptr_array[15];

/* Function pointer with complex signature */
typedef struct base_struct* (*GTY(()) factory_func)(int, const char*);

/* Undefined type forward declaration */
struct GTY(()) undefined_struct;
typedef struct undefined_struct GTY(()) undefined_struct_t;

#endif /* GTY_TEST_TYPES_H */
