/* test_gty.h - Comprehensive GTY annotation test file */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) __attribute__((gty_annotate x))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* Expected: TYPE_UNDEFINED */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;
/* Expected: TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
const char * GTY((length(strlen(my_string)))) my_string;
/* Expected: TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct * GTY((skip)) next;
};
/* Expected: TYPE_STRUCT */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};
/* Expected: TYPE_UNION */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
/* Expected: TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
int GTY((length("10"))) my_array[10];
/* Expected: TYPE_ARRAY */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
/* Expected: TYPE_CALLBACK */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("tree_common"), desc("tree_code"))) lang_struct_base {
    tree code;
    union GTY((desc("%1.code"))) lang_union {
        int a;
        tree b;
    } u;
};

struct GTY(()) lang_struct_extended {
    struct lang_struct_base base;
    rtx rtx_field;
    gimple gimple_field;
};
/* Expected: TYPE_LANG_STRUCT */

/* ========== Additional test cases for coverage ========== */

/* Nested struct with pointer chain */
struct GTY(()) nested_container {
    struct my_struct inner;
    union my_union choice;
    int GTY((length("5"))) dynamic_array[5];
};

/* Variable length array with callback */
typedef struct GTY(()) var_len_struct {
    int length;
    int GTY((length("%h.length"))) data[];
} var_len_struct_t;

/* Chain of pointers */
typedef struct GTY(()) pointer_chain {
    struct pointer_chain * GTY((skip)) next;
    struct pointer_chain * GTY((skip)) prev;
    void * GTY((skip)) data;
} pointer_chain_t;

/* Mixed types in union */
union GTY((desc("%d.type"))) mixed_union {
    struct {
        int type;
        int value;
    } GTY((tag("0"))) int_pair;
    struct {
        int type;
        char * GTY((length(strlen(str)))) str;
    } GTY((tag("1"))) string_pair;
};

/* Array of pointers */
struct my_struct * GTY((length("3"))) pointer_array[3];

/* String array */
const char * GTY((length("2"))) string_array[2];

#endif /* TEST_GTY_H */
