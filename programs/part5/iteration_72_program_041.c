#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

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
    struct base_struct *GTY((skip)) struct_ptr;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) user_def_struct {
    long counter;
    union data_union data;
} user_def_struct_t;

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    user_def_struct_t *GTY((skip)) data;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_matrix[10][20];

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) event_callback)(int event_id, void *GTY((skip)) user_data);

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) timestamp;
    double GTY((skip)) value;
};

/* String type - TYPE_STRING */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Nested type structure for complex traversal */
struct GTY(()) complex_type {
    struct linked_node *GTY((skip)) node_list;
    union data_union variant;
    int_matrix matrix;
    event_callback callback;
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
struct GTY(()) options_test {
    int GTY((tag("0"), skip)) tag_field;
    struct linked_node *GTY((chain_next, skip)) chain_node;
    void *GTY((skip, desc("1"))) desc_ptr;
};

/* Recursive type definition */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

/* Array of pointers */
typedef struct base_struct *GTY(()) struct_ptr_array[50];

/* Union containing structure pointer */
union GTY(()) union_with_struct {
    struct base_struct *GTY((skip)) sptr;
    struct linked_node *GTY((skip)) nptr;
    event_callback callback;
};

/* Simulating lang_struct pattern (TYPE_LANG_STRUCT) */
struct GTY(()) lang_simul {
    int lang_specific;
    struct GTY((skip)) lang_simul *next;
};

/* Forward declarations for cross-file references */
struct GTY(()) forward_declared;
typedef struct forward_declared forward_declared_t;

/* Undefined type reference */
extern struct undefined_type *GTY((skip)) undefined_ptr;

#endif /* GTY_TEST_TYPES_H */
