/* test_gty.h - Comprehensive test of GTY annotations for gengtype coverage */

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

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Use dummy GCC type */
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) *tree_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback_fn)(tree, rtx);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure full traversal */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct my_struct *first;
    struct GTY(()) inner_struct {
        int data;
        struct inner_struct *next;
    } *inner;
};

/* Variable length array */
struct GTY(()) varray_struct {
    int length;
    int GTY((length("%0.length"))) data[];
};

/* Union with tag */
union GTY((tag("UNION_TAG"))) tagged_union {
    int i;
    double d;
    char *str;
};

/* Chain of pointers */
typedef struct GTY(()) chain_node {
    int value;
    struct chain_node * GTY((skip)) next;
} chain_node_t;

/* Array of pointers */
chain_node_t * GTY((length)) node_array[20];

/* Callback in struct */
struct GTY(()) callback_container {
    my_callback_fn callback;
    tree_callback_fn tree_callback;
};

#endif /* TEST_GTY_H */
