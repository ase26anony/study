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

/* ===== TYPE_UNDEFINED ===== */
/* Forward declaration without definition */
struct GTY(()) my_undefined_struct;

/* ===== TYPE_SCALAR ===== */
typedef int GTY((user)) my_scalar_t;

/* ===== TYPE_STRING ===== */
const char * GTY((length)) my_string;

/* ===== TYPE_STRUCT ===== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ===== TYPE_UNION ===== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Use dummy GCC type */
};

/* ===== TYPE_POINTER ===== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) *tree_pointer;

/* ===== TYPE_ARRAY ===== */
int GTY((length)) my_array[10];
tree GTY((length)) tree_array[5];

/* ===== TYPE_CALLBACK ===== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback_fn)(tree, rtx);

/* ===== TYPE_LANG_STRUCT ===== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union GTY((desc("lang_code"))) {
        tree c_tree;
        rtx c_rtx;
        gimple c_gimple;
    } GTY((tag("0"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) complex_container {
    /* Nested struct */
    struct GTY((tag("nested"))) nested_struct {
        int id;
        char * GTY((length)) name;
    } nested;
    
    /* Pointer member */
    struct nested_struct * GTY((skip)) ptr;
    
    /* Array member */
    int GTY((length)) counts[20];
    
    /* Union member */
    union GTY((desc("id"))) {
        int as_int;
        char * GTY((skip)) as_string;
    } data;
    
    /* Callback member */
    my_callback_fn GTY((user)) callback;
};

/* Global variables with various GTY annotations */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern tree GTY((user)) global_tree;
extern rtx GTY((skip)) global_rtx;

#endif /* TEST_GTY_H */
