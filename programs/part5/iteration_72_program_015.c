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

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;  /* Recursive pointer */
    struct list_node *GTY((skip)) prev;  /* Another pointer */
    union data_union data;
    int_array numbers;  /* Array member */
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;
typedef struct list_node GTY(()) list_node_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
typedef const char *GTY(()) const_string;

/* Scalar type container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) long_value;
    unsigned GTY((skip)) uint_value;
    double GTY((skip)) double_value;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct list_node *GTY((skip)) head;
    union data_union current;
    struct scalar_container scalars;
    callback_func handler;
    const_string description;
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

/* Structure with multiple GTY options chained */
struct GTY(()) multi_option_struct {
    int id;
    /* Chain multiple options: skip and potentially others */
    void *GTY((skip)) opaque_data;
    struct list_node *GTY((skip)) node_ptr;
};

/* Forward declaration for recursive structures */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of different GTY-marked types */
struct GTY(()) heterogeneous_array {
    struct base_struct structs[3];
    union data_union unions[2];
    struct list_node *GTY((skip)) pointers[4];
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_struct types often have special handling */
struct GTY(()) lang_simulated {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

#endif /* GTY_TEST_TYPES_H */
