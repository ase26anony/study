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

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Pointer type - TYPE_POINTER */
typedef struct base_struct *GTY(()) base_struct_ptr;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    double GTY((skip)) double_value;
};

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Recursive structure for complex type graphs */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union data_union *GTY((skip)) data;
};

/* Structure containing union */
struct GTY(()) struct_with_union {
    int type;
    union {
        int int_member;
        float float_member;
        struct base_struct *GTY((skip)) struct_member;
    } GTY((tag("type"))) value;  /* Using tag option */
};

/* Union containing structure */
union GTY(()) union_with_struct {
    struct base_struct GTY(()) base;
    struct tree_node GTY(()) node;
    int_array numbers;
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
struct GTY(()) complex_options {
    int GTY((skip, tag("1"))) field1;
    void *GTY((skip, length("field1"))) field2;  /* Simulating length option */
    struct base_struct *GTY((skip)) field3;
};

/* Array with length option simulation */
struct GTY(()) variable_array {
    int count;
    struct base_struct *GTY((skip, length("count"))) items;
};

/* For TYPE_LANG_STRUCT simulation - using a special naming pattern */
struct GTY(()) lang_struct_sim {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Typedef creating various type kinds */
typedef union data_union GTY(()) data_union_t;
typedef struct tree_node GTY(()) tree_node_t;
typedef int GTY(()) gty_int_t;

#endif /* GTY_TEST_TYPES_H */
