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
const char * GTY((length("strlen(%h.%s)"))) my_string;

/* Another string example */
char * GTY((length)) dynamic_string;

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct with tag - should be TYPE_STRUCT */
struct GTY((tag("my_struct_tag"))) my_struct {
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
/* User-defined struct type - should be TYPE_USER_STRUCT */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* Another user struct example */
typedef struct outer_struct GTY((user)) outer_user_t;

/* ==================== TYPE_UNION ==================== */
/* Union with desc attribute - should be TYPE_UNION */
union GTY((desc("%0.a"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* Union with nested struct */
union GTY((desc("%1.type"))) complex_union {
    struct {
        int type;
        int data;
    } GTY((tag("struct_part"))) s;
    char * GTY((skip)) str;
};

/* ==================== TYPE_POINTER ==================== */
/* Pointer to struct - should be TYPE_POINTER */
struct my_struct * GTY((skip)) my_pointer;

/* Pointer chain */
struct my_struct ** GTY((skip)) double_pointer;

/* Pointer in struct */
struct GTY((tag("ptr_struct"))) ptr_struct {
    struct my_struct * GTY((skip)) ptr_field;
    int count;
};

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size array - should be TYPE_ARRAY */
int GTY((length("10"))) my_array[10];

/* Array of pointers */
struct my_struct * GTY((length("5"))) ptr_array[5];

/* Array in struct */
struct GTY((tag("array_struct"))) array_struct {
    int GTY((length("8"))) values[8];
    char name[32];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer type - should be TYPE_CALLBACK */
typedef void (*GTY((user)) my_callback_fn)(int, char*);

/* Callback in struct */
struct GTY((tag("callback_struct"))) callback_struct {
    my_callback_fn GTY((skip)) handler;
    void * GTY((skip)) data;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure - should be TYPE_LANG_STRUCT */
struct GTY((special("lang_struct"), desc("%0.type"))) lang_specific {
    int type;
    union {
        tree t;
        rtx r;
        gimple g;
    } GTY((desc("%0.type"))) u;
};

/* Another lang struct example for C++ frontend pattern */
struct GTY((special("tree_common"))) tree_common {
    int code;
    union {
        struct lang_specific * GTY((skip)) lang;
        int value;
    } GTY((desc("%0.code"))) u;
};

/* ==================== Mixed Examples ==================== */
/* Struct containing multiple types */
struct GTY((tag("comprehensive"))) comprehensive {
    /* Scalar */
    my_scalar_t scalar_field;
    
    /* String */
    char * GTY((length)) string_field;
    
    /* Pointer */
    struct my_struct * GTY((skip)) ptr_field;
    
    /* Array */
    int GTY((length("4"))) array_field[4];
    
    /* Union */
    union my_union union_field;
    
    /* Nested struct */
    struct array_struct nested_struct;
    
    /* Callback */
    my_callback_fn GTY((skip)) callback_field;
};

/* Global variables with various types */
extern struct my_struct GTY((tag("global"))) global_struct;
extern union my_union GTY((desc("%0.a"))) global_union;
extern int GTY((length)) global_array[];
extern struct lang_specific * GTY((skip)) global_lang_ptr;

#endif /* TEST_GTY_H */
