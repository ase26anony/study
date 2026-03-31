/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

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
/* Expected: nb_undefined++ */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;
/* Expected: nb_scalar++ */

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;
/* Expected: nb_string++ */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree GTY((skip)) tree_field;
};
/* Expected: nb_struct++ */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: nb_user_struct++ */

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx GTY((tag("rtx"))) r;
};
/* Expected: nb_union++ */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((user)) tree_pointer;
/* Expected: nb_pointer++ */

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) struct_array[5];
/* Expected: nb_array++ */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback)(rtx, int);
/* Expected: nb_callback++ */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%0.lang_code"))) u;
};
/* Expected: nb_lang_struct++ */

/* Additional complex types to ensure full traversal */
struct GTY(()) container {
    /* Nested pointer */
    struct container * GTY((skip)) next;
    
    /* Array of pointers */
    tree * GTY((length)) trees[8];
    
    /* Union with desc */
    union GTY((desc("%0.type"))) data {
        int i;
        char * GTY((length)) str;
    } d;
    
    int type;
};

/* Variable-length array with length attribute */
struct GTY(()) varray {
    int GTY((length)) count;
    int GTY((length)) items[1];
};

/* Chain of structures for testing traversal */
struct GTY(()) chain {
    int id;
    struct chain * GTY((skip)) next;
    struct chain * GTY((skip)) prev;
};

/* Test multiple inheritance-like pattern */
struct GTY((tag("base"))) base_struct {
    int base_field;
};

struct GTY((tag("derived"))) derived_struct {
    struct base_struct GTY((skip)) base;
    int derived_field;
};

/* Test callback in struct */
struct GTY(()) callback_container {
    my_callback_fn GTY((skip)) callback;
    void (* GTY((user)) another_callback)(struct callback_container *);
};

#endif /* TEST_GTY_H */
