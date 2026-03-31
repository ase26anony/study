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
struct GTY(()) my_undefined_struct;

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string_var;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree GTY((skip)) field2;  /* Using dummy GCC type */
    char * GTY((length)) field3;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((tag("1"))) c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((user)) tree_pointer;  /* Pointer to GCC type */

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((tag("array_struct"))) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) gcc_callback_fn)(rtx, gimple);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_id;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%1.lang_id"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) nested_struct {
    struct my_struct GTY((tag("inner"))) inner;
    union my_union GTY((desc("2"))) u;
    my_callback_fn GTY((skip)) callback;
};

/* Variable declarations using the types */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern my_scalar_t GTY(()) global_scalar;

/* Function pointer in struct */
struct GTY(()) funcptr_struct {
    my_callback_fn GTY((user)) handler;
    gcc_callback_fn GTY((user)) gcc_handler;
};

/* Array of pointers */
struct my_struct * GTY((length)) ptr_array[8];

/* Self-referential structure */
struct GTY((tag("self_ref"))) self_ref_struct {
    int data;
    struct self_ref_struct * GTY((skip)) next;
};

#endif /* TEST_GTY_H */
