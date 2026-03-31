#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Base GTY-marked structure */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this field */
};

/* GTY-marked union */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((tag("1"))) struct_ptr;
};

/* Array type with GTY */
typedef int GTY(()) int_array[10];

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;

/* Function pointer (callback) type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Recursive structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
};

/* Structure containing union */
struct GTY(()) container {
    int type;
    union data_union GTY((tag("0"))) data;
};

/* Pointer-only structure */
struct GTY(()) pointer_chain {
    struct base_struct *GTY((skip)) next_base;
    struct container *GTY((skip)) next_container;
    callback_func GTY((skip)) callback;
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct base_struct *GTY((skip)) items[5];
    int_array numbers;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
};
#else
struct linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
};
#endif

/* Simulating lang_struct-like behavior */
struct GTY(()) lang_type {
    int lang_specific;
    struct GTY((skip)) base_struct *base;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) static_string;
    char *GTY((skip)) dynamic_string;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    double GTY((skip)) double_value;
    unsigned GTY((skip)) flags;
};

/* Complex nested type */
struct GTY(()) complex_type {
    struct container GTY((tag("0"))) container_field;
    union data_union GTY((tag("1"))) union_field;
    struct pointer_array *GTY((skip)) ptr_array;
    callback_func GTY((skip)) handlers[3];
};

#endif /* GTY_TEST_TYPES_H */
