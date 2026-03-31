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
struct GTY(()) undefined_struct;

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree nested_tree;  /* Use dummy GCC type */
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
tree * GTY((user)) tree_pointer;

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
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct * GTY((skip)) next;
    } *inner;
    my_callback_fn callback;
};

/* Variable-length array in struct */
struct GTY(()) var_struct {
    int count;
    tree GTY((length("%0.count"))) items[];
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct {
        int type;
        tree value;
    } s;
    rtx insn;
};

/* Template-like pattern (common in GCC) */
#define DEFTREECODE(SYM, STRING, TYPE, NARGS)   \
  struct GTY(()) SYM ## _node {                 \
    tree common;                                \
    tree operands[NARGS];                       \
  };

/* Use the macro to generate some tree nodes */
DEFTREECODE(ERROR_MARK, "error_mark", 'e', 0)
DEFTREECODE(IDENTIFIER_NODE, "identifier_node", 'i', 0)

/* GTY-rooted variable declarations */
extern struct my_struct GTY(()) global_struct;
extern tree GTY((length)) global_tree_vec[];
extern union my_union GTY(()) global_union;

/* Function with GTY parameters */
void process_tree(tree GTY((skip)) t, rtx GTY((user)) r);

#endif /* TEST_GTY_H */
