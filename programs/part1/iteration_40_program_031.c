/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
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
tree * GTY((chain_next("tree"), chain_prev("tree"))) tree_chain;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback)(rtx);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) nested_struct {
    struct my_struct *ptr;
    union my_union data;
    int GTY((skip)) skipped_field;
};

/* Variable length array with length argument */
struct GTY(()) var_len_struct {
    int length;
    int GTY((length("%h.length"))) data[];
};

/* Parametrized type */
typedef struct GTY(()) param_struct {
    enum { PARAM_A, PARAM_B } param_type;
    union {
        int a;
        double b;
    } GTY((desc("%0.param_type"))) value;
} param_t;

/* Chain of pointers */
struct GTY((chain_next("%h.next"))) chain_node {
    int value;
    struct chain_node *next;
};

#endif /* TEST_GTY_H */
