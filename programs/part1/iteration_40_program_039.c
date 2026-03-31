/* test_gty.h - Comprehensive test of GTY annotations for gengtype coverage */

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
    char * GTY((skip)) name;
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
tree GTY((skip)) tree_pointer;
rtx GTY((skip)) rtx_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) struct_array[5];
char * GTY((length)) string_array[3];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef int (*GTY((user)) another_callback)(tree, rtx);

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

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct * GTY((skip)) next;
    } inner;
    struct inner_struct * GTY((skip)) chain;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct {
        int type;
        char * GTY((length)) data;
    } s;
    struct my_struct * GTY((skip)) ptr;
};

/* Array of pointers */
tree * GTY((length)) tree_ptr_array[8];

/* Struct with callback field */
struct GTY(()) has_callback {
    int id;
    my_callback_fn GTY((skip)) callback;
};

/* Forward declared struct that gets defined later */
struct GTY(()) late_defined_struct;

/* Actually define it now */
struct GTY(()) late_defined_struct {
    int value;
    struct late_defined_struct * GTY((skip)) next;
};

/* Variable declarations using our types */
extern struct my_struct GTY((skip)) global_struct;
extern union my_union GTY((skip)) global_union;
extern my_callback_fn GTY((skip)) global_callback;

/* Typedef combinations */
typedef struct my_struct * GTY((user)) my_struct_ptr_t;
typedef union my_union GTY((user)) my_union_t;

#endif /* TEST_GTY_H */
