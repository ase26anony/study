/* test_gty.h - Comprehensive GTY annotation test header */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (as in GCC build) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
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
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;  /* TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
    int field;
    tree node;  /* Using dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Using dummy GCC type */
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */
rtx GTY((skip)) my_rtx_pointer;  /* Another pointer with GCC type */

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];  /* TYPE_ARRAY */
tree GTY((length)) tree_array[5];  /* Array of GCC types */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure pattern */
struct GTY((special("lang_struct"))) lang_specific_struct {  /* TYPE_LANG_STRUCT */
    int lang_code;
    union GTY((desc("0"))) {
        tree t;
        rtx r;
        gimple g;
    } GTY((tag("0"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) nested_example {
    struct my_struct *GTY((skip)) ptr_field;
    union my_union GTY((desc("1"))) union_field;
    int GTY((length)) dynamic_array[];
};

/* Variable declarations using our types */
extern my_scalar_t GTY((user)) global_scalar;
extern struct my_struct GTY((tag("global_struct"))) global_struct_var;
extern union my_union GTY((desc("2"))) global_union_var;

/* Function pointer in a struct */
struct GTY(()) funcptr_container {
    my_callback_fn GTY((user)) callback;
    void (*GTY((user)) another_callback)(struct my_struct*);
};

/* Array of pointers */
struct my_struct * GTY((length)) ptr_array[8];

/* Nested struct with array */
struct GTY(()) container {
    struct nested_example GTY((tag("nested"))) nested;
    int GTY((length)) matrix[3][3];
};

#endif /* TEST_GTY_H */
