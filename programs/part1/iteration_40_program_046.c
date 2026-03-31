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
extern struct undefined_struct *GTY(()) undefined_ptr;
/* Expected: TYPE_UNDEFINED */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;
/* Expected: TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
const char * GTY((length(strlen("test")))) my_string;
/* Expected: TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct *GTY((skip)) next;
};
/* Expected: TYPE_STRUCT */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct *GTY((skip)) c;
};
/* Expected: TYPE_UNION */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((skip)) tree_pointer;
rtx GTY((skip)) rtx_pointer;
/* Expected: TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
int GTY((length("10"))) my_array[10];
struct my_struct GTY((length("5"))) struct_array[5];
/* Expected: TYPE_ARRAY */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_var;
/* Expected: TYPE_CALLBACK */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure pattern */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%1.lang_code"))) u;
};
/* Expected: TYPE_LANG_STRUCT */

/* ========== Additional complex types for thorough testing ========== */

/* Nested struct with union */
struct GTY(()) container {
    struct my_struct GTY((skip)) embedded;
    union my_union GTY((skip)) data;
    int GTY((length("container_count"))) *dynamic_array;
};

/* Pointer to array */
int (*GTY((skip)) array_ptr)[10];

/* Function pointer with callback */
typedef int (*GTY((user)) compare_func)(const void*, const void*);
compare_func GTY((skip)) sorter;

/* Chain of structures */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int value;
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
};

/* Variable length array in struct */
struct GTY(()) var_struct {
    int count;
    int GTY((length("%h.count"))) items[];
};

/* Union with desc based on field */
union GTY((desc("%0.type"))) typed_union {
    int type;
    struct my_struct GTY((tag("1"))) s;
    union my_union GTY((tag("2"))) u;
};

#endif /* TEST_GTY_H */
