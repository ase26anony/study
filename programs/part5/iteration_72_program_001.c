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
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;  /* Recursive pointer */
    struct linked_node *GTY((skip)) prev;  /* Another pointer */
    union data_union data;
    int_array scores;  /* Array type field */
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, struct base_struct*);

/* Structure containing callback */
struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void *GTY((skip)) user_data;
};

/* Nested type structure */
struct GTY(()) outer_struct {
    struct linked_node *GTY((skip)) node_list;
    union data_union current_data;
    struct {
        int x;
        int y;
    } GTY((skip)) coordinates;  /* Anonymous struct */
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int special_field;
    struct outer_struct *GTY((skip)) outer;
};
#else
struct conditional_struct {
    int special_field;
    struct outer_struct *outer;
};
#endif

/* String type handling - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    unsigned GTY((skip)) flags;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* Complex chain of types */
struct GTY(()) complex_node {
    struct complex_node *GTY((skip)) children[4];  /* Array of pointers */
    union data_union payload;
    callback_func GTY((skip)) cleanup;
};

/* For simulating lang_struct-like behavior */
struct GTY(()) tree_node_sim {
    int code;
    union {
        long int_val;
        double real_val;
        struct complex_node *GTY((skip)) complex_ptr;
    } GTY((skip)) u;
};

/* Multiple GTY options chained */
struct GTY(()) option_test {
    void *GTY((skip, tag("special"))) tagged_ptr;
    int GTY((length("len_field"))) variable_array[1];
    int len_field;
};

#endif /* GTY_TEST_TYPES_H */
