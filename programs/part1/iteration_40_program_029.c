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
struct GTY(()) my_undefined_struct;

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length(strlen(my_string)))) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct * GTY((skip)) next;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a ? 0 : 1"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((ptr_alias(1))) *my_tree_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length("my_array_len"))) my_array[10];
extern int my_array_len;

struct GTY(()) array_container {
    int GTY((length("count"))) *items;
    int count;
};

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef int (*GTY((user)) compare_fn)(const void *, const void *);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"), desc("%0.type"))) lang_specific {
    int type;
    union GTY((desc("%0.type"))) {
        int ival;
        double dval;
        char * GTY((length("strlen(%h.sval)+1"))) sval;
    } u;
};

/* Another approach for lang struct */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_node {
    struct lang_node *next;
    struct lang_node *prev;
    tree value;
};

/* ========== Additional complex types for thorough testing ========== */

/* Nested struct with union */
struct GTY(()) complex_type {
    int tag;
    union GTY((desc("%0.tag"))) {
        struct GTY((tag("sub1"))) {
            int x;
            int y;
        } s1;
        struct GTY((tag("sub2"))) {
            double d;
            char * GTY((length("strlen(%h.str)+1"))) str;
        } s2;
    } u;
};

/* Variable length array in struct */
struct GTY(()) varray {
    int length;
    int GTY((length("%h.length"))) data[1];
};

/* Pointer chain */
struct GTY((chain_next("%h.next"))) pointer_chain {
    int value;
    struct pointer_chain *next;
};

/* Use of callback type */
struct GTY(()) callback_container {
    my_callback_fn GTY((skip)) callback;
    void * GTY((skip)) user_data;
};

/* Array of pointers */
tree * GTY((length("num_trees"))) tree_array[5];
extern int num_trees;

/* Union with nested struct */
union GTY((desc("%0.type"))) nested_union {
    int type;
    struct GTY((tag("nested"))) {
        int a;
        int b;
    } nested;
};

/* String with callback for length */
const char * GTY((length("get_string_length(%h)"))) dynamic_string;
int get_string_length(const char *str);

#endif /* TEST_GTY_H */
