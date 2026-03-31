/* test_gty.h - Comprehensive GTY annotation test header */
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
/* This should trigger TYPE_UNDEFINED when processed */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length("strlen(%h.my_string)"))) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree GTY((skip)) tree_field;  /* Using dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a ? 0 : 1"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
rtx GTY((skip)) *rtx_pointer;  /* Pointer to dummy GCC type */

/* ========== TYPE_ARRAY ========== */
int GTY((length("10"))) my_array[10];
struct my_struct GTY((length("5"))) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_ptr;

/* ========== TYPE_LANG_STRUCT ========== */
/* Create a language-specific structure pattern */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%h.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct * GTY((skip)) next;
    } * GTY((skip)) inner;
    
    union my_union GTY((skip)) u;
};

/* Variable-length array */
struct GTY(()) varray_struct {
    int count;
    int GTY((length("%h.count"))) variable_array[];
};

/* Chain of pointers */
typedef struct GTY(()) chain_node {
    int value;
    struct chain_node * GTY((skip)) next;
} chain_node_t;

chain_node_t * GTY((skip)) chain_head;

/* Function pointer in struct */
struct GTY(()) func_container {
    void (*GTY((user)) func)(void);
    int data;
};

/* Multiple GTY attributes */
struct GTY((tag("complex"), skip)) complex_struct {
    int GTY((user)) scalar_field;
    const char * GTY((length("strlen(%h.str_field)"))) str_field;
    struct complex_struct * GTY((skip)) recursive_ptr;
};

#endif /* TEST_GTY_H */
