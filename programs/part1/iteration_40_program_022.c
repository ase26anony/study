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

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* This should trigger TYPE_UNDEFINED when processed */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;  /* Expected: TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
const char * GTY((length(strlen("%s") + 1))) my_string;  /* Expected: TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct_tag"))) my_struct {  /* Expected: TYPE_STRUCT */
    int field1;
    tree GTY((skip)) field2;  /* Skip this field */
    const char * GTY((length)) field3;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* Expected: TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union GTY((desc("%d"))) my_union {  /* Expected: TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((tag("my_struct_ptr"))) c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;  /* Expected: TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
int GTY((length("sizeof(%s)/sizeof(%s[0])"))) my_array[10];  /* Expected: TYPE_ARRAY */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);  /* Expected: TYPE_CALLBACK */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"), chain_next("%s.next"), chain_prev("%s.prev"))) lang_specific_struct {
    int lang_field;
    union my_union GTY((desc("0"))) lang_union;
    struct lang_specific_struct *next;
    struct lang_specific_struct *prev;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) nested_struct {
    struct my_struct inner;
    union my_union GTY((desc("1"))) nested_union;
};

/* Pointer array */
struct my_struct * GTY((length("5"))) pointer_array[5];

/* Callback in struct */
struct GTY(()) callback_container {
    my_callback_fn GTY((user)) callback;
    int data;
};

/* Variable length array marker */
struct GTY(()) var_len_struct {
    int length;
    int GTY((length("%s.length"))) flexible_array[];
};

/* Nested pointer chain */
struct GTY(()) chain_struct {
    int value;
    struct chain_struct * GTY((skip)) next;
};

/* Union with nested struct */
union GTY((desc("%d"))) complex_union {
    struct my_struct s;
    struct nested_struct ns;
    rtx GTY((skip)) r;
};

#endif /* TEST_GTY_H */
