/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
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
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree GTY((skip)) field2;  /* Using dummy GCC type */
    const char* GTY((length)) field3;
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
rtx GTY((user)) *my_rtx_pointer;  /* Pointer to dummy GCC type */

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((tag("array_elem"))) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) gcc_callback_fn)(rtx, gimple);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_tag;
    union {
        tree GTY((tag("0"))) expr;
        rtx GTY((tag("1"))) insn;
    } GTY((desc("%1.lang_tag"))) u;
};

/* Additional complex types to ensure thorough parsing */

/* Nested struct with pointer chain */
struct GTY(()) outer_struct {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct * GTY((skip)) next;
    } *inner;
    
    union my_union variant;
};

/* Variable-length array */
struct GTY(()) var_len_struct {
    int length;
    int GTY((length("%0.length"))) flexible_array[];
};

/* Parametrized type */
typedef struct my_struct* GTY((user)) my_struct_ptr;

/* Multiple pointers in one declaration */
tree GTY((user)) *ptr1, *ptr2;

#endif /* TEST_GTY_H */
