#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this field */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((tag("0"))) ptr_val;  /* Tagged pointer */
    struct base_struct *GTY((skip)) struct_ptr;
};

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union data;
    int GTY((skip)) private_data;
};

/* Array type - TYPE_ARRAY */
typedef struct base_struct GTY(()) struct_array[10];
typedef int GTY(()) int_matrix[5][5];

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) event_callback)(int event_id, void *GTY((skip)) user_data);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned int GTY((skip)) flags;
    double GTY((skip)) value;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Complex nested structure */
struct GTY(()) complex_type {
    struct linked_node *GTY((skip)) node_list;
    union data_union GTY((skip)) variant;
    struct_array items;
    event_callback GTY((skip)) callback;
    struct complex_type *GTY((skip)) self_ref;  /* Recursive pointer */
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

/* Array with length specifier */
struct GTY(()) variable_array {
    int count;
    struct base_struct *GTY((length("count"))) elements;  /* Variable length array */
};

/* Multiple GTY options */
struct GTY(()) multi_option_struct {
    void *GTY((skip, tag("1"))) tagged_ptr;
    int GTY((skip)) hidden_field;
    const char *GTY((skip)) constant_string;
};

/* Simulating lang_struct-like behavior */
struct GTY(()) tree_like_struct {
    int code;
    union {
        long int_val;
        double real_val;
        struct tree_like_struct *GTY((skip)) child;
    } GTY((skip)) u;
};

/* Undefined type forward declaration */
struct GTY(()) undefined_struct;

/* Function pointer with complex signature */
typedef int (*GTY(()) compare_func)(
    const void *GTY((skip)) a,
    const void *GTY((skip)) b,
    void *GTY((skip)) context
);

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */
