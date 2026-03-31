/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((gty_annotate x))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;

/* ============================================
   TYPE_SCALAR: Basic scalar type with user annotation
   ============================================ */
typedef int GTY((user)) my_scalar_t;

/* ============================================
   TYPE_STRING: String with length attribute
   ============================================ */
const char * GTY((length("strlen(%h.my_string)"))) my_string;

/* ============================================
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct_tag"))) my_struct {
    int field1;
    tree field2;  /* Using dummy GCC type */
    struct my_struct *next GTY((skip));
};

/* ============================================
   TYPE_USER_STRUCT: User-defined struct type
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ============================================
   TYPE_UNION: Union with desc attribute for discriminant
   ============================================ */
union GTY((desc("%0.a"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
    struct my_struct *d;
};

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct * GTY((skip)) my_struct_pointer;
tree * GTY((user)) my_tree_pointer;
rtx GTY((user)) my_rtx_pointer;

/* ============================================
   TYPE_ARRAY: Arrays with length attributes
   ============================================ */
int GTY((length("sizeof(%h.my_fixed_array)/sizeof(%h.my_fixed_array[0])"))) 
    my_fixed_array[10];

struct my_struct * GTY((length("my_var_len"))) 
    my_var_len_array[];

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, tree);
typedef int (*GTY((user)) another_callback)(struct my_struct *);

/* ============================================
   TYPE_LANG_STRUCT: Language-specific structure
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        tree expr;
        rtx insn;
    } GTY((desc("0"))) u;
};

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY(()) complex_node {
    int data;
    struct complex_node * GTY((skip)) next;
    struct complex_node * GTY((skip)) prev;
    union my_union value;
};

/* Array of pointers */
tree * GTY((length("num_trees"))) tree_array[];

/* Struct containing callback */
struct GTY(()) callback_container {
    my_callback_fn handler GTY((skip));
    int priority;
};

/* Forward declaration that will be defined later */
struct GTY(()) forward_declared;

/* The actual definition */
struct GTY(()) forward_declared {
    int defined_now;
    struct forward_declared *link;
};

/* Global variables with GTY annotations for root table generation */
extern struct my_struct GTY(()) global_struct;
extern tree GTY(()) global_tree;
extern rtx GTY(()) global_rtx;
extern struct my_lang_struct GTY(()) global_lang_struct;

#endif /* TEST_GTY_H */
