/* test_gty.h - Comprehensive test of GTY annotations for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) undefined_struct;  /* Expected: TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR: Basic scalar type with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* Expected: TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length attribute
   ============================================ */
const char * GTY((length(strlen("test")))) my_string;  /* Expected: TYPE_STRING */

/* ============================================
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* Expected: TYPE_STRUCT */
    int field;
    my_scalar_t scalar_field;
};

/* ============================================
   TYPE_USER_STRUCT: Typedef of struct with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* Expected: TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with descriminator
   ============================================ */
union GTY((desc("0"))) my_union {  /* Expected: TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    struct my_struct *c;
};

/* ============================================
   TYPE_POINTER: Pointer to struct with skip attribute
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* Expected: TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("10"))) my_array[10];  /* Expected: TYPE_ARRAY */

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* Expected: TYPE_CALLBACK */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific structure
   ============================================ */
/* Create a struct that mimics GCC's language-specific structures */
struct GTY((special("lang_struct"))) lang_specific_struct {  /* Expected: TYPE_LANG_STRUCT */
    int lang_code;
    union GTY((desc("lang_code"))) {
        struct my_struct *c_struct;
        tree c_tree;
        rtx c_rtx;
    } GTY((tag("0"))) u;
};

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY(()) complex_struct {
    struct my_struct *first;
    struct GTY((tag("nested"))) nested {
        int count;
        my_callback_fn callback;
    } inner;
    union my_union choice;
};

/* Array of pointers */
struct my_struct * GTY((length("5"))) ptr_array[5];

/* Pointer to array */
int (*GTY((skip)) ptr_to_array)[10];

/* Self-referential struct */
struct GTY((tag("self_ref"))) self_ref_struct {
    int value;
    struct self_ref_struct * GTY((skip)) next;
};

/* Union containing various types */
union GTY((desc("type"))) variant_union {
    int type;
    struct my_struct * GTY((tag("1"))) as_struct;
    tree GTY((tag("2"))) as_tree;
    rtx GTY((tag("3"))) as_rtx;
    my_callback_fn GTY((tag("4"))) as_callback;
};

#endif /* TEST_GTY_H */
