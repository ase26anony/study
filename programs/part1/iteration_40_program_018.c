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
    tree nested_tree;  /* Use dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
rtx GTY((skip)) my_rtx_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_ptr;

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_id;
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_id"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) complex_container {
    /* Mix of different types */
    my_scalar_t scalar_field;
    struct my_struct * GTY((skip)) ptr_field;
    int GTY((length)) array_field[20];
    union my_union union_field;
};

/* Nested pointer chain */
struct GTY(()) nested_pointers {
    struct complex_container * GTY((skip)) level1;
    struct nested_pointers * GTY((skip)) next;
};

/* Template-like structure (common in GCC) */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int value;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
};

/* Variable length array with length specifier */
struct GTY(()) var_len_struct {
    int length;
    int GTY((length("%h.length"))) data[];
};

/* Union with tag for discrimination */
union GTY((desc("%d.type"))) tagged_union {
    struct {
        int type;
        int value;
    } GTY((tag("0"))) int_val;
    struct {
        int type;
        double value;
    } GTY((tag("1"))) double_val;
};

#endif /* TEST_GTY_H */
