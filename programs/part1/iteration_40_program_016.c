/* test_gty.h - Test file for gengtype coverage of type statistics */
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

/* ===== TYPE_UNDEFINED ===== */
/* Forward declaration without definition */
struct GTY(()) my_undefined_struct;

/* ===== TYPE_SCALAR ===== */
typedef int GTY((user)) my_scalar_t;

/* ===== TYPE_STRING ===== */
const char * GTY((length)) my_string;

/* ===== TYPE_STRUCT ===== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ===== TYPE_UNION ===== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Use dummy GCC type */
};

/* ===== TYPE_POINTER ===== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((chain_next("tree"), chain_prev("tree"))) tree_chain;

/* ===== TYPE_ARRAY ===== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ===== TYPE_CALLBACK ===== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback_fn)(tree, rtx);

/* ===== TYPE_LANG_STRUCT ===== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) nested_container {
    struct my_struct GTY((tag("nested"))) nested;
    union my_union variant;
    struct lang_specific_struct *lang_ptr;
};

/* Variable declarations using the types */
extern my_scalar_t global_scalar GTY((user));
extern struct my_struct global_struct GTY((tag("global")));
extern union my_union global_union GTY((desc("1")));
extern my_callback_fn callback_var GTY((user));

#endif /* TEST_GTY_H */
