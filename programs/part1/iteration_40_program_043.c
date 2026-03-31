/* test_gty.h - Header with GTY annotations covering all type categories */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined (as in GCC build system) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* This should be counted as TYPE_UNDEFINED */

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar type with user annotation */
typedef int GTY((user)) my_scalar_t;

/* ==================== TYPE_STRING ==================== */
/* String type with length attribute */
const char * GTY((length(strlen("%s") + 1))) my_string;

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct with tag attribute */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* Another struct for variety */
struct GTY(()) another_struct {
    int x;
    double y;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ==================== TYPE_UNION ==================== */
/* Union with desc attribute for discriminant */
union GTY((desc("%0.a"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* Another union with nested structure */
union GTY(()) complex_union {
    int int_val;
    double double_val;
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((skip)) tree_pointer;
rtx GTY((skip)) rtx_pointer;

/* Pointer in a struct */
struct GTY(()) pointer_container {
    int id;
    struct my_struct * GTY((skip)) ptr;
};

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size array */
int GTY((length("10"))) my_array[10];

/* Variable-length array in struct */
struct GTY(()) array_container {
    int count;
    int GTY((length("%h.count"))) values[];
};

/* Array of pointers */
tree * GTY((length("5"))) tree_array[5];

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer type */
typedef void (*GTY((user)) my_callback_fn)(int);

/* Callback in struct */
struct GTY(()) callback_container {
    int id;
    my_callback_fn GTY((skip)) callback;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure - using special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_specific_field;
    union {
        tree t;
        rtx r;
    } GTY((desc("%0.lang_specific_field"))) u;
};

/* Another approach: struct with nested language union */
struct GTY(()) tree_common {
    tree chain;
    tree type;
    enum tree_code {
        ERROR_MARK,
        IDENTIFIER_NODE,
        /* ... other codes ... */
    } code;
};

/* GCC's tree structure pattern */
struct GTY((chain_next("%h.chain"), chain_prev("%h.chain"))) tree_decl {
    struct tree_common common;
    const char * GTY((skip)) name;
};

/* ==================== Additional types for completeness ==================== */

/* Struct containing various types */
struct GTY(()) composite_type {
    my_scalar_t scalar;
    struct my_struct * GTY((skip)) pointer;
    int GTY((length("8"))) array[8];
    union my_union data;
};

/* Root variable declarations */
extern struct my_struct GTY(()) global_struct;
extern tree GTY((length("100"))) global_tree_array[100];
extern rtx GTY((skip)) global_rtx;

#endif /* TEST_GTY_H */
