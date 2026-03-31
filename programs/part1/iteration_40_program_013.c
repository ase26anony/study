/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) __attribute__((gty_annotate))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration without definition - should be TYPE_UNDEFINED */
struct GTY(()) undefined_struct;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar type with user annotation - should be TYPE_SCALAR */
typedef int GTY((user)) my_scalar_t;

/* Another scalar example */
typedef long GTY((user)) my_long_t;

/* ==================== TYPE_STRING ==================== */
/* String type with length attribute - should be TYPE_STRING */
const char * GTY((length(strlen("test") + 1))) my_string;

/* Another string example */
char * GTY((length)) dynamic_string;

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct with tag - should be TYPE_STRUCT */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    char field2;
    my_scalar_t field3;
};

/* Nested struct example */
struct GTY((tag("outer_struct"))) outer_struct {
    int id;
    struct my_struct GTY((tag("nested"))) nested;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User struct type - should be TYPE_USER_STRUCT */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct */
typedef struct outer_struct GTY((user)) my_outer_struct_t;

/* ==================== TYPE_UNION ==================== */
/* Union with desc attribute - should be TYPE_UNION */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct GTY((tag("union_struct"))) s;
    union my_union u;
    int array[5];
};

/* ==================== TYPE_POINTER ==================== */
/* Simple pointer - should be TYPE_POINTER */
struct my_struct * GTY((skip)) my_pointer;

/* Pointer to pointer */
struct my_struct ** GTY((skip)) my_double_pointer;

/* Pointer in struct */
struct GTY((tag("ptr_struct"))) ptr_struct {
    struct my_struct * GTY((skip)) ptr_field;
    union my_union * GTY((skip)) union_ptr;
};

/* ==================== TYPE_ARRAY ==================== */
/* Fixed array - should be TYPE_ARRAY */
int GTY((length("10"))) my_array[10];

/* Array of pointers */
struct my_struct * GTY((length("5"))) ptr_array[5];

/* Array in struct */
struct GTY((tag("array_struct"))) array_struct {
    int GTY((length("20"))) data[20];
    char GTY((length("50"))) buffer[50];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer type - should be TYPE_CALLBACK */
typedef void (*GTY((user)) my_callback_fn)(int);

/* Callback with parameters */
typedef int (*GTY((user)) process_fn)(const char *, int);

/* Callback in struct */
struct GTY((tag("callback_struct"))) callback_struct {
    my_callback_fn GTY((skip)) callback;
    process_fn GTY((skip)) processor;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific struct - should be TYPE_LANG_STRUCT */
struct GTY((special("lang_struct"))) lang_specific_struct {
    tree node;
    rtx insn;
    gimple stmt;
    int lang_specific_field;
};

/* Another lang struct with nested union */
struct GTY((special("tree_node"))) tree_node_struct {
    tree base;
    union {
        int ival;
        double dval;
        char *sval;
    } GTY((desc("0"))) u;
};

/* ==================== COMPLEX EXAMPLES ==================== */
/* Mixed type example to ensure full traversal */
struct GTY((tag("complex_example"))) complex_example {
    /* Scalar field */
    my_scalar_t scalar_field;
    
    /* String field */
    const char * GTY((length)) name;
    
    /* Pointer field */
    struct my_struct * GTY((skip)) data_ptr;
    
    /* Array field */
    int GTY((length("100"))) values[100];
    
    /* Union field */
    union my_union GTY((desc("0"))) choice;
    
    /* Callback field */
    my_callback_fn GTY((skip)) handler;
    
    /* Nested struct */
    struct array_struct nested_array_struct;
    
    /* Pointer to lang struct */
    struct lang_specific_struct * GTY((skip)) lang_data;
};

/* Global variables with various types */
extern struct my_struct GTY((tag("global_struct"))) global_var;
extern union my_union GTY((desc("0"))) global_union;
extern int GTY((length("100"))) global_array[100];
extern struct complex_example * GTY((skip)) global_complex_ptr;

#endif /* TEST_GTY_H */
