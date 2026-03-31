/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

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
tree GTY((ptr_alias)) tree_ptr;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback)(rtx);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY(()) complex_struct {
    struct my_struct *first;
    struct GTY((tag("nested"))) nested {
        int count;
        tree GTY((length)) items[8];
    } data;
    my_callback_fn callback;
};

/* Variable-length array in struct */
struct GTY(()) var_struct {
    int length;
    int GTY((length("%0.length"))) elements[];
};

/* Union with nested struct */
union GTY((desc("1"))) nested_union {
    struct {
        int type;
        tree value;
    } s;
    char *name;
};

/* Chain of pointers */
typedef struct chain_node GTY(()) chain_node;
struct GTY(()) chain_node {
    int value;
    chain_node *GTY((skip)) next;
};

/* Array of pointers */
tree * GTY((length)) ptr_array[20];

/* String array */
const char * GTY((length)) string_array[] = {"one", "two", "three"};

/* Function pointer with arguments */
typedef void (*GTY((user)) event_handler)(tree source, rtx data, int flags);

/* Complete example using all features */
struct GTY((tag("example"))) example_container {
    my_scalar_t id;
    const char * GTY((length)) name;
    struct my_struct data;
    union my_union variant;
    tree GTY((length)) nodes[5];
    struct example_container *GTY((skip)) next;
    event_handler on_event;
};

#endif /* TEST_GTY_H */
