/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* Expected: nb_undefined++ */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;
/* Expected: nb_scalar++ */

/* ========== TYPE_STRING ========== */
const char * GTY((length(strlen(my_string)))) my_string;
/* Expected: nb_string++ */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree GTY((skip)) tree_field;  /* Using dummy GCC type */
};
/* Expected: nb_struct++ */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: nb_user_struct++ */

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx GTY((tag("RTX"))) rtx_field;  /* Using dummy GCC type */
};
/* Expected: nb_union++ */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) tree_chain;
/* Expected: nb_pointer++ (multiple times) */

/* ========== TYPE_ARRAY ========== */
int GTY((length("10"))) my_array[10];
tree GTY((length("5"))) tree_array[5];
/* Expected: nb_array++ (multiple times) */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback_fn)(tree, tree);
/* Expected: nb_callback++ (multiple times) */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"), desc("%1.type"))) lang_specific_struct {
    int type;
    union {
        int int_val;
        tree GTY((tag("TREE"))) tree_val;
    } GTY((desc("0"))) u;
};
/* Expected: nb_lang_struct++ */

/* Additional complex types to ensure full traversal */
struct GTY(()) complex_container {
    my_scalar_t scalar;
    my_user_struct_t *user_struct_ptr;
    my_callback_fn callback;
    struct undefined_struct *undefined_ptr;
    union my_union union_field;
    int GTY((length("dynamic_length"))) dynamic_array[];
};

/* Nested structure with multiple GTY attributes */
struct GTY((tag("outer"))) outer_struct {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct *next;
    } *list;
    
    /* Array of pointers */
    struct inner_struct * GTY((length("count"))) *ptr_array;
    int count;
};

/* Variable-length structure */
struct GTY((variable_size)) var_size_struct {
    int length;
    int GTY((length("%0.length"))) data[1];
};

#endif /* TEST_GTY_H */
