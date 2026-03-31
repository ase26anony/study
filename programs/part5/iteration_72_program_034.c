#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type with GTY marker */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this pointer during GC */
};

/* Union type with GTY marker */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Recursive structure - linked list node */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;  /* Pointer to same type */
    struct list_node *GTY((skip)) prev;  /* Another pointer */
};

/* Structure containing a union */
struct GTY(()) container {
    int type;
    union data_union GTY((tag("type"))) value;  /* Use tag option */
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct list_node *GTY(()) node_ptr_array[5];

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;

/* Function pointer (callback) type */
typedef void (*GTY(()) callback_func)(int, void*);

/* Structure with callback field */
struct GTY(()) event_handler {
    callback_func GTY((skip)) handler;  /* Skip function pointer */
    void *GTY((skip)) user_data;
};

/* Nested structure */
struct GTY(()) outer_struct {
    int outer_id;
    struct GTY(()) inner_struct {
        int inner_id;
        char *GTY((skip)) inner_name;
    } inner;
    struct inner_struct *GTY((skip)) inner_ptr;
};

/* For TYPE_LANG_STRUCT simulation - using a special naming pattern */
struct GTY(()) lang_specific {
    int lang_type;
    void *GTY((skip)) lang_data;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int conditional_field;
    char *GTY((skip)) conditional_ptr;
};
#else
struct conditional_struct {
    int conditional_field;
    char *conditional_ptr;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) complex_options {
    int GTY((skip)) ignored_field;
    struct list_node *GTY((skip, length("len"))) nodes;
    int len;
    callback_func GTY((skip)) callbacks[3];
};

/* Scalar type in container */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;
    unsigned long GTY((skip)) another_scalar;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) mutable_string;
};

/* Forward declarations for cross-file references */
struct GTY(()) forward_declared;
extern struct forward_declared *GTY((skip)) global_forward_ptr;

#endif /* GTY_TEST_TYPES_H */
