/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (as in GCC build environment) */
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

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;  /* Expected: TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
const char * GTY((length("strlen(%h.my_string)"))) my_string;  /* Expected: TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {  /* Expected: TYPE_STRUCT */
    int field;
    tree node;  /* Use dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* Expected: TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a ? 0 : 1"))) my_union {  /* Expected: TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((tag("my_struct"))) c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;  /* Expected: TYPE_POINTER */
rtx GTY((user)) *my_rtx_pointer;  /* Another pointer type */

/* ========== TYPE_ARRAY ========== */
int GTY((length("my_array_len"))) my_array[10];  /* Expected: TYPE_ARRAY */
struct my_struct GTY((length("2"))) struct_array[2];  /* Array of structs */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);  /* Expected: TYPE_CALLBACK */
typedef tree (*GTY((user)) tree_callback)(rtx, gimple);  /* Another callback */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure - often has nested union with GTY markers */
struct GTY((special("lang_struct"))) lang_specific_struct {  /* Expected: TYPE_LANG_STRUCT */
    int lang_code;
    union GTY((desc("%1.lang_code"))) {
        struct my_struct * GTY((tag("my_struct"))) s;
        rtx GTY((skip)) r;
        tree GTY((user)) t;
    } GTY((skip)) u;
};

/* Complex nested structure to exercise more parsing paths */
struct GTY(()) container {
    /* Mix of different types */
    my_scalar_t scalar_field;
    const char * GTY((length("strlen(%h.string_field)"))) string_field;
    struct my_struct GTY((tag("my_struct"))) struct_field;
    union my_union GTY((desc("%h.union_field.a ? 0 : 1"))) union_field;
    struct my_struct * GTY((skip)) pointer_field;
    int GTY((length("5"))) array_field[5];
    my_callback_fn callback_field;
    struct lang_specific_struct GTY((special("lang_struct"))) lang_field;
    
    /* Pointer to undefined type */
    struct undefined_struct * GTY((skip)) undefined_ptr;
};

/* Global variables with GTY annotations */
extern struct container GTY(()) global_container;
extern tree GTY((user)) global_tree;
extern rtx GTY((skip)) *global_rtx_array;

#endif /* TEST_GTY_H */
