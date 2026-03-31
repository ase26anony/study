/* test_gty.h - Comprehensive GTY annotation test header */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) 
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
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree GTY((skip)) field2;  /* Using dummy GCC type */
    char * GTY((length)) field3;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((tag("1"))) c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
rtx GTY((user)) *my_rtx_pointer;  /* Pointer to dummy GCC type */

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((tag("array_struct"))) my_struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_ptr;

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct * GTY((skip)) next;
    } *inner;
    my_callback_fn GTY((user)) handler;
};

/* Variable length array */
struct GTY(()) var_len_struct {
    int count;
    int GTY((length("%0.count"))) items[];
};

/* Union with nested struct */
union GTY((desc("0"))) complex_union {
    struct {
        int type;
        char * GTY((length)) name;
    } s;
    struct my_struct * GTY((tag("1"))) p;
};

/* Template-like pattern (common in GCC) */
#define DEF_STRUCT(name, field_type) \
    struct GTY(()) name { \
        field_type GTY((skip)) field; \
    }

DEF_STRUCT(generated_struct, tree);

/* Chain of pointers */
typedef struct GTY(()) chain_node {
    int value;
    struct chain_node * GTY((skip)) next;
} chain_node_t;

chain_node_t * GTY((skip)) chain_head;

#endif /* TEST_GTY_H */
