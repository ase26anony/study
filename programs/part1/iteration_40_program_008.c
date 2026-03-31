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
struct GTY(()) undefined_struct;

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
tree * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) tree_chain;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback)(rtx, gimple);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%0.lang_code"))) u;
};

/* ========== Additional complex types for thorough testing ========== */

/* Nested struct with pointer field */
struct GTY(()) outer_struct {
    struct my_struct * GTY((skip)) inner;
    union my_union data;
};

/* Variable length array */
struct GTY(()) varray_struct {
    int length;
    int GTY((length("%0.length"))) items[];
};

/* Chain of structures */
struct GTY((chain_next("%h.next"))) chain_node {
    int value;
    struct chain_node *next;
};

/* Use all the types to ensure they're processed */
extern my_scalar_t global_scalar;
extern const char * GTY((length)) global_string;
extern struct my_struct global_struct;
extern my_user_struct_t global_user_struct;
extern union my_union global_union;
extern struct my_struct * GTY((skip)) global_pointer;
extern int GTY((length)) global_array[20];
extern my_callback_fn global_callback;
extern struct lang_specific_struct global_lang_struct;

#endif /* TEST_GTY_H */
