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
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((tag("0"))) struct_ptr;
};

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;  /* Recursive pointer */
    struct list_node *GTY((skip)) prev;
    union data_union data;
    int priority;
};

/* Array type - TYPE_ARRAY */
typedef struct list_node *GTY(()) node_array[100];

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned long GTY((skip)) total;
    double GTY((skip)) average;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Complex nested structure */
struct GTY(()) complex_type {
    struct base_struct base;
    union data_union variant;
    struct list_node *GTY((skip)) head;
    node_array nodes;  /* Array of pointers */
    callback_func GTY((skip)) handler;
    struct scalar_container stats;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    float y;
    struct conditional_struct *GTY((skip)) self_ptr;
};
#else
struct conditional_struct {
    int x;
    float y;
    struct conditional_struct *self_ptr;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) options_test {
    int GTY((skip, tag("1"))) field1;
    void *GTY((skip, chain_next, chain_prev)) chain_field;
    struct list_node *GTY((length("10"))) node_list[10];
};

/* Simulating lang_struct-like behavior */
struct GTY((for_user)) user_lang_struct {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_child;

struct GTY(()) tree_node {
    int value;
    struct tree_child *GTY((skip)) first_child;
    struct tree_node *GTY((skip)) parent;
};

struct GTY(()) tree_child {
    struct tree_node *GTY((skip)) node;
    struct tree_child *GTY((skip)) next;
};

/* Undefined type reference (should trigger TYPE_UNDEFINED) */
struct undefined_type *GTY((skip)) undefined_ptr;

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */
