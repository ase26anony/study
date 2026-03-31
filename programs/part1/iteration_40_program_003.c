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
typedef long GTY((user)) my_long_t;

/* ========== TYPE_STRING ========== */
/* String type with length attribute */
const char * GTY((length(strlen("test")))) my_string;

/* Another string example */
char * GTY((length)) dynamic_string;

/* ========== TYPE_STRUCT ========== */
/* Regular struct with tag */
struct GTY((tag("my_struct_tag"))) my_struct {
    int field1;
    char * GTY((skip)) field2;
    tree field3;  /* Using dummy GCC type */
};

/* Nested struct example */
struct GTY((tag("outer_struct"))) outer_struct {
    int id;
    struct my_struct GTY((tag("nested"))) nested;
};

/* ========== TYPE_USER_STRUCT ========== */
/* User-defined struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct */
typedef struct outer_struct GTY((user)) outer_user_t;

/* ========== TYPE_UNION ========== */
/* Union with desc attribute for discriminant */
union GTY((desc("$0"))) my_union {
    int GTY((tag("0"))) a;
    char * GTY((tag("1"), skip)) b;
    struct my_struct GTY((tag("2"))) c;
};

/* Another union example */
union GTY((desc("type"))) variant_union {
    int GTY((tag("INT"))) int_val;
    double GTY((tag("DOUBLE"))) double_val;
    char * GTY((tag("STRING"), length)) string_val;
};

/* ========== TYPE_POINTER ========== */
/* Simple pointer */
struct my_struct * GTY((skip)) my_pointer;

/* Pointer to union */
union my_union * GTY((skip)) union_ptr;

/* Pointer chain */
struct my_struct ** GTY((skip)) double_ptr;

/* ========== TYPE_ARRAY ========== */
/* Fixed-size array */
int GTY((length("10"))) my_array[10];

/* Array of pointers */
struct my_struct * GTY((length("5"))) ptr_array[5];

/* Multi-dimensional array */
int GTY((length("3*4"))) matrix[3][4];

/* ========== TYPE_CALLBACK ========== */
/* Function pointer type */
typedef void (*GTY((user)) my_callback_fn)(int, char*);

/* Another callback with different signature */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);

/* Struct containing callback */
struct GTY((tag("callback_container"))) callback_container {
    my_callback_fn GTY((skip)) callback;
    int data;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure - using special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_id;
    union {
        tree GTY((tag("TREE"))) t;
        rtx GTY((tag("RTX"))) r;
    } GTY((desc("$0"))) u;
    void (* GTY((skip)) lang_method)(void);
};

/* Another language struct pattern */
struct GTY((tag("gcc_lang_struct"))) gcc_lang_struct {
    int code;
    struct GTY((tag("base"))) {
        int flags;
        tree type;
    } base;
    union GTY((desc("$0.code"))) {
        tree GTY((tag("0"))) expr;
        rtx GTY((tag("1"))) insn;
        gimple GTY((tag("2"))) stmt;
    } GTY((skip)) u;
};

/* ========== Complex nested example ========== */
/* This should exercise multiple type categories */
struct GTY((tag("complex_example"))) complex_example {
    /* Scalar */
    my_scalar_t scalar_field;
    
    /* Pointer */
    struct lang_specific_struct * GTY((skip)) lang_ptr;
    
    /* Array */
    my_callback_fn GTY((length("3"))) callbacks[3];
    
    /* Union */
    union my_union data;
    
    /* Nested struct */
    struct {
        int nested_scalar;
        char * GTY((length)) nested_string;
    } GTY((tag("anonymous"))) nested;
};

/* ========== Forward references that become defined later ========== */
struct GTY(()) late_defined_struct;

/* Later definition */
struct GTY((tag("late_defined"))) late_defined_struct {
    int value;
    struct late_defined_struct * GTY((skip)) next;
};

#endif /* TEST_GTY_H */
