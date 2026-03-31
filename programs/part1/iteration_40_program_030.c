/* test_gty.h - Comprehensive test of GTY annotations for gengtype coverage */

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
tree GTY((user)) *tree_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY(()) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((user)) callback_var;

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree expr;
        rtx pattern;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) nested_struct {
    struct my_struct *ptr;
    union my_union data;
    int GTY((length)) dynamic_array[];
};

/* Variable-length structure */
struct GTY((variable_size)) var_size_struct {
    int length;
    int GTY((length("%h.length"))) data[];
};

/* Chain structure for linked list testing */
struct GTY((chain_next("%h.next"))) chain_struct {
    int value;
    struct chain_struct * GTY((skip)) next;
};

/* Parametric structure */
struct GTY((param_is(struct my_struct))) param_struct {
    void *data;
};

#endif /* TEST_GTY_H */
