/* test_gty.h - Comprehensive test of GTY annotations for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) undefined_struct;

/* TYPE_SCALAR: Basic scalar type with user annotation */
typedef int GTY((user)) my_scalar_t;

/* TYPE_STRING: String with length attribute */
const char * GTY((length(strlen("test")))) my_string;

/* TYPE_STRUCT: Regular struct with tag */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* TYPE_UNION: Union with desc attribute for discriminant */
union GTY((desc("$1"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Use dummy GCC type */
};

/* TYPE_POINTER: Pointer with skip attribute */
struct my_struct * GTY((skip)) my_pointer;

/* TYPE_ARRAY: Array with length attribute */
int GTY((length("10"))) my_array[10];

/* TYPE_CALLBACK: Function pointer with user annotation */
typedef void (*GTY((user)) my_callback_fn)(int);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((special("lang_struct"))) lang_specific {
    int lang_code;
    union {
        tree ast_node;
        rtx rtl_insn;
        gimple gimple_stmt;
    } GTY((desc("lang_code"))) u;
};

/* Additional complex types to ensure full traversal */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct my_struct *first;
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct *next;
    } *inner;
};

/* Variable length array in struct */
struct GTY(()) varray_struct {
    int count;
    int GTY((length("count"))) elements[];
};

/* Chain of pointers */
typedef struct chain_node GTY(()) chain_node_t;
struct GTY(()) chain_node {
    int value;
    chain_node_t *next;
};

/* Union with nested struct */
union GTY((desc("$2"))) complex_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        char *name;
        int id;
    } GTY((tag("person"))) person;
};

#endif /* TEST_GTY_H */
