/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

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
    tree nested;  /* Use dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    tree c;  /* Use dummy GCC type */
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
rtx GTY((user)) *rtx_ptr;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback_fn)(tree, rtx);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union GTY((desc("%1.lang_code"))) {
        tree tree_part;
        rtx rtx_part;
        gimple gimple_part;
    } GTY((tag("0"))) u;
};

/* Additional complex types to ensure full traversal */
struct GTY(()) container {
    /* Nested pointer */
    struct lang_specific_struct * GTY((skip)) lang_ptr;
    
    /* Array of pointers */
    tree * GTY((length)) ptr_array[8];
    
    /* Callback field */
    my_callback_fn GTY((user)) callback;
    
    /* Union field */
    union my_union GTY((desc("0"))) data;
};

/* Variable declarations with GTY annotations */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern tree GTY((user)) global_tree;

#endif /* TEST_GTY_H */
