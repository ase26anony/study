/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

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
/* Forward declaration without definition - should be TYPE_UNDEFINED */
struct GTY(()) undefined_struct;

/* ========== TYPE_SCALAR ========== */
/* Basic scalar type with user annotation */
typedef int GTY((user)) my_scalar_t;

/* Another scalar example */
typedef long GTY((user)) my_long_t;

/* ========== TYPE_STRING ========== */
/* String type with length attribute */
const char * GTY((length(strlen("test")))) my_string;

/* Another string example */
char * GTY((length)) dynamic_string;

/* ========== TYPE_STRUCT ========== */
/* Regular struct with tag */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    char field2;
    tree field3;  /* Using dummy GCC type */
};

/* Nested struct example */
struct GTY((tag("outer_struct"))) outer_struct {
    int id;
    struct my_struct GTY((tag("nested"))) nested;
};

/* ========== TYPE_USER_STRUCT ========== */
/* User struct type definition */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct */
typedef struct outer_struct GTY((user)) my_outer_struct_t;

/* ========== TYPE_UNION ========== */
/* Union with desc attribute for discrimination */
union GTY((desc("$1"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((tag("struct_ptr"))) c;
};

/* Nested union in struct */
struct GTY((tag("union_container"))) union_container {
    int type;
    union GTY((desc("type"))) {
        int int_val;
        char * GTY((length)) str_val;
        double dbl_val;
    } value;
};

/* ========== TYPE_POINTER ========== */
/* Simple pointer */
struct my_struct * GTY((skip)) my_pointer;

/* Pointer to pointer */
struct my_struct ** GTY((skip)) my_double_pointer;

/* Pointer in struct */
struct GTY((tag("pointer_struct"))) pointer_struct {
    int count;
    struct my_struct * GTY((skip)) items;
};

/* ========== TYPE_ARRAY ========== */
/* Fixed-size array */
int GTY((length("10"))) my_array[10];

/* Array of pointers */
struct my_struct * GTY((length("5"))) pointer_array[5];

/* Flexible array member */
struct GTY((tag("flex_array"))) flex_array {
    int length;
    int GTY((length("$0.length"))) data[];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer type */
typedef void (*GTY((user)) my_callback_fn)(int);

/* Callback in struct */
struct GTY((tag("callback_struct"))) callback_struct {
    int id;
    my_callback_fn GTY((skip)) callback;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure pattern */
struct GTY((special("lang_struct"), desc("1"))) lang_specific_struct {
    int lang_code;
    union GTY((desc("$0.lang_code"))) {
        tree c_tree;
        rtx c_rtx;
        gimple c_gimple;
    } u;
};

/* Another lang struct example with chain */
struct GTY((special("lang_struct"), chain_next("$0.next"))) lang_chain_struct {
    int value;
    struct lang_chain_struct *next;
};

/* ========== Mixed/Complex Examples ========== */
/* Struct containing multiple types */
struct GTY((tag("complex_example"))) complex_example {
    /* Scalar */
    my_scalar_t scalar_field;
    
    /* String */
    char * GTY((length)) name;
    
    /* Pointer */
    struct my_struct * GTY((skip)) ptr_field;
    
    /* Array */
    int GTY((length("count"))) scores[10];
    
    /* Union */
    union GTY((desc("type"))) {
        int int_val;
        char * GTY((length)) str_val;
    } data;
    
    /* Callback */
    my_callback_fn GTY((skip)) handler;
    
    /* Nested struct */
    struct my_struct nested;
};

/* Global variables for testing */
extern struct my_struct GTY((tag("global_struct"))) global_var;
extern union my_union GTY((desc("0"))) global_union;
extern int GTY((length)) global_array[20];

#endif /* TEST_GTY_H */
