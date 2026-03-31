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
typedef int gimple;

/* ===== TYPE_UNDEFINED ===== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;

/* ===== TYPE_SCALAR ===== */
typedef int GTY((user)) my_scalar_t;

/* ===== TYPE_STRING ===== */
const char * GTY((length)) my_string;

/* ===== TYPE_STRUCT ===== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree nested_tree;  /* Use dummy GCC type */
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ===== TYPE_UNION ===== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx c;  /* Use dummy GCC type */
};

/* ===== TYPE_POINTER ===== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((user)) tree_pointer;

/* ===== TYPE_ARRAY ===== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ===== TYPE_CALLBACK ===== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_ptr;

/* ===== TYPE_LANG_STRUCT ===== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific {
    int lang_id;
    union GTY((desc("0"))) {
        tree t;
        rtx r;
    } GTY((tag("0"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct my_struct * GTY((skip)) ptr1;
    struct GTY(()) inner_struct {
        int data;
        struct inner_struct * GTY((skip)) next;
    } * GTY((skip)) chain;
};

/* Variable length array */
struct GTY(()) varray_struct {
    int count;
    int GTY((length("%0.count"))) data[];
};

/* Union with callback */
union GTY((desc("1"))) union_with_callback {
    int value;
    my_callback_fn GTY((skip)) callback;
};

/* Array of pointers */
tree * GTY((length)) ptr_array[8];

/* String array */
const char * GTY((length)) string_array[] = {"test1", "test2", "test3"};

/* Complete the undefined struct */
struct undefined_struct {
    int defined_now;
    struct my_struct * GTY((skip)) link;
};

#endif /* TEST_GTY_H */
