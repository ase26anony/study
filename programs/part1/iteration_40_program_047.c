/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((gty_annotate))
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
const char * GTY((length("strlen(%h.my_string)"))) my_string;
/* Expected: TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    char * GTY((skip)) field2;
    struct my_struct *next;
};
/* Expected: TYPE_STRUCT */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a ? 0 : 1"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};
/* Expected: TYPE_UNION */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) my_tree_pointer;
/* Expected: TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
int GTY((length("sizeof(%h.my_array)/sizeof(%h.my_array[0])"))) my_array[10];
struct my_struct GTY((length("5"))) struct_array[5];
/* Expected: TYPE_ARRAY */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef int (*GTY((user)) another_callback)(tree, rtx);
/* Expected: TYPE_CALLBACK */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with nested union */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%0.lang_code"))) u;
    void (*lang_method)(void);
};
/* Expected: TYPE_LANG_STRUCT */

/* Additional complex types to ensure thorough traversal */

/* Nested struct with pointer chain */
struct GTY(()) complex_struct {
    struct my_struct *first;
    union my_union data;
    my_callback_fn callback;
    struct GTY((tag("nested"))) nested {
        int x;
        struct nested *next;
    } *nested_ptr;
};

/* Variable-length array in struct */
struct GTY(()) var_struct {
    int count;
    int GTY((length("%h.count"))) items[];
};

/* Chain of pointers */
typedef struct chain_node GTY(()) chain_node_t;
struct GTY(()) chain_node {
    int value;
    chain_node_t * GTY((skip)) next;
};

/* Self-referential structure */
struct GTY((tag("self_ref"))) self_ref {
    int id;
    struct self_ref * GTY((skip)) child;
};

/* Union with struct members */
union GTY((desc("%0.type"))) mixed_union {
    int type;
    struct {
        int x, y;
    } GTY((tag("point"))) point;
    struct {
        char *name;
        int age;
    } GTY((tag("person"))) person;
};

#endif /* TEST_GTY_H */
