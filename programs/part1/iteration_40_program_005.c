/* test_gty.h - Comprehensive GTY annotation test file
 * This file contains examples of all GTY type categories to exercise
 * the statistics collection function in gengtype.cc
 */

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

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* Expected: nb_undefined++ */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;
/* Expected: nb_scalar++ */

/* ========== TYPE_STRING ========== */
const char * GTY((length("strlen(%h.my_string)"))) my_string;
/* Expected: nb_string++ */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct *GTY((skip)) next;
};
/* Expected: nb_struct++ */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: nb_user_struct++ */

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a ? 0 : 1"))) my_union {
    int GTY((tag("0"))) a;
    char * GTY((tag("1"))) b;
    struct my_struct * GTY((tag("2"))) c;
};
/* Expected: nb_union++ */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
/* Expected: nb_pointer++ */

/* ========== TYPE_ARRAY ========== */
int GTY((length("10"))) my_array[10];
/* Expected: nb_array++ */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
/* Expected: nb_callback++ */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
        gimple GTY((tag("2"))) g;
    } GTY((desc("%h.lang_code"))) u;
};
/* Expected: nb_lang_struct++ */

/* ========== Additional test structures ========== */

/* Nested structure with multiple pointer types */
struct GTY(()) container {
    struct my_struct *GTY((skip)) ptr1;
    struct lang_specific_struct *GTY((skip)) ptr2;
    my_callback_fn GTY((user)) callback;
    int GTY((user)) count;
    char * GTY((length("strlen(%h.data)"))) data;
};

/* Array of pointers */
struct my_struct * GTY((length("%h.count"))) ptr_array[];

/* Union with nested structures */
union GTY((desc("%0.type"))) complex_union {
    struct {
        int type;
        struct my_struct GTY((tag("1"))) s;
    } GTY((tag("0"))) a;
    struct {
        int type;
        union my_union GTY((tag("1"))) u;
    } GTY((tag("1"))) b;
};

/* Chain of structures for linked list testing */
struct GTY(()) node {
    int value;
    struct node *GTY((skip)) next;
    struct node *GTY((skip)) prev;
};

/* Test variable declarations using the types */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct lang_specific_struct GTY(()) global_lang_struct;
extern struct container GTY(()) global_container;

#endif /* TEST_GTY_H */
