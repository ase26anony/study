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
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* This should trigger TYPE_UNDEFINED when first encountered */

/* ========== TYPE_SCALAR ========== */
/* Basic scalar type with user annotation */
typedef int GTY((user)) my_scalar_t;

/* Another scalar example */
typedef unsigned long GTY((user)) my_ulong;

/* ========== TYPE_STRING ========== */
/* String type with length annotation */
const char * GTY((length("strlen(%h.my_string)"))) my_string;

/* Another string example */
char * GTY((length)) dynamic_string;

/* ========== TYPE_STRUCT ========== */
/* Regular struct with tag annotation */
struct GTY((tag("my_struct_tag"))) my_struct {
    int field1;
    char field2;
    my_scalar_t field3;
};

/* Nested struct example */
struct GTY((tag("outer_struct"))) outer_struct {
    int id;
    struct my_struct GTY((tag("nested_field"))) nested;
};

/* ========== TYPE_USER_STRUCT ========== */
/* User-defined struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct example */
typedef struct outer_struct GTY((user)) outer_user_t;

/* ========== TYPE_UNION ========== */
/* Union with desc annotation for discriminant */
union GTY((desc("$a ? 1 : 0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct GTY((tag("union_struct"))) c;
};

/* Another union example */
union GTY((desc("0"))) simple_union {
    int int_val;
    double double_val;
    const char* GTY((length)) str_val;
};

/* ========== TYPE_POINTER ========== */
/* Simple pointer with skip annotation */
struct my_struct * GTY((skip)) my_pointer;

/* Pointer chain */
struct my_struct ** GTY((skip)) double_pointer;

/* Pointer to union */
union my_union * GTY((skip)) union_pointer;

/* ========== TYPE_ARRAY ========== */
/* Fixed-size array with length annotation */
int GTY((length("10"))) my_array[10];

/* Variable-length array in struct */
struct GTY((tag("array_struct"))) array_container {
    int count;
    int GTY((length("%h.count"))) variable_array[1];
};

/* Array of pointers */
struct my_struct * GTY((length("5"))) pointer_array[5];

/* ========== TYPE_CALLBACK ========== */
/* Function pointer type with user annotation */
typedef void (*GTY((user)) my_callback_fn)(int);

/* Callback in struct */
struct GTY((tag("callback_struct"))) callback_container {
    int id;
    my_callback_fn GTY((skip)) callback;
};

/* Another callback example */
typedef int (*GTY((user)) compare_func)(const void*, const void*);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure pattern */
struct GTY((special("lang_struct"))) lang_specific {
    int lang_code;
    union {
        tree GTY((tag("tree_node"))) t;
        rtx GTY((tag("rtx_def"))) r;
        gimple GTY((tag("gimple_statement"))) g;
    } GTY((desc("%h.lang_code"))) u;
};

/* Another lang struct example mimicking GCC's tree_common */
struct GTY((special("tree_common"))) tree_common_struct {
    union {
        struct GTY((tag("tree_base"))) base;
        int code;
    } GTY((desc("0"))) u;
    tree GTY((chain_next("%h"))) chain;
};

/* ========== Complex example combining multiple types ========== */
struct GTY((tag("complex_example"))) complex_type {
    /* Scalar field */
    my_scalar_t scalar_field;
    
    /* String field */
    const char* GTY((length)) name;
    
    /* Pointer field */
    struct complex_type* GTY((skip)) next;
    
    /* Union field */
    union my_union data;
    
    /* Array field */
    int GTY((length("count"))) scores[10];
    int count;
    
    /* Callback field */
    my_callback_fn GTY((skip)) handler;
    
    /* Nested struct */
    struct array_container array_data;
};

/* ========== Now define the previously undefined struct ========== */
/* This should now be TYPE_STRUCT, not TYPE_UNDEFINED */
struct GTY((tag("now_defined"))) undefined_struct {
    int defined_field;
    struct my_struct* GTY((skip)) link;
};

/* Global variables with various types for root table generation */
extern struct my_struct GTY((root)) global_struct;
extern union my_union GTY((root)) global_union;
extern struct complex_type* GTY((root)) global_complex;

#endif /* TEST_GTY_H */
