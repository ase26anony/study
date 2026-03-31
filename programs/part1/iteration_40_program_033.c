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
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct *next;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) my_tree_pointer;
rtx GTY((user)) my_rtx_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) my_struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((user)) current_callback;

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree t;
        rtx r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) complex_nested {
    struct GTY((tag("inner"))) inner_struct {
        int data;
        struct inner_struct *next;
    } inner;
    
    union GTY((desc("0"))) nested_union {
        int ival;
        char *sval;
    } u;
    
    int GTY((length)) dynamic_array[];
};

/* Template-like structure with conditional fields */
struct GTY(()) conditional_struct {
    int type;
    union GTY((desc("%0.type"))) {
        struct my_struct *s;
        my_callback_fn fn;
        int * GTY((length)) arr;
    } data;
};

/* Chain of pointers for pointer chasing */
struct GTY(()) pointer_chain {
    int value;
    struct pointer_chain * GTY((skip)) next;
    struct pointer_chain * GTY((user)) prev;
};

/* Array of pointers */
struct my_struct * GTY((length)) pointer_array[20];

/* String array */
const char * GTY((length)) string_array[] = {"test1", "test2", "test3"};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        char *name;
        int id;
    } info;
};

#endif /* TEST_GTY_H */
