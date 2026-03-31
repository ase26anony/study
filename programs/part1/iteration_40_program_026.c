/* test_gty.h - Test header for gengtype coverage */
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
struct GTY(()) undefined_struct;

/* ===== TYPE_SCALAR ===== */
typedef int GTY((user)) my_scalar_t;

/* ===== TYPE_STRING ===== */
const char * GTY((length)) my_string;

/* ===== TYPE_STRUCT ===== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    char * GTY((skip)) name;
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ===== TYPE_UNION ===== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* ===== TYPE_POINTER ===== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((skip)) tree_pointer;

/* ===== TYPE_ARRAY ===== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) struct_array[5];

/* ===== TYPE_CALLBACK ===== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((skip)) callback_ptr;

/* ===== TYPE_LANG_STRUCT ===== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%1.lang_code"))) u;
};

/* Additional complex types to ensure thorough parsing */
struct GTY(()) complex_container {
    struct GTY((tag("nested"))) nested_struct {
        int x;
        char * GTY((length)) str;
    } nested;
    
    union my_union GTY((skip)) u;
    
    struct lang_specific_struct * GTY((skip)) lang_ptr;
    
    /* Array of pointers */
    struct my_struct * GTY((length)) ptr_array[8];
};

/* Variable declarations using the types */
extern struct my_struct GTY((skip)) global_struct;
extern union my_union GTY((skip)) global_union;
extern struct undefined_struct * GTY((skip)) undefined_ptr;

#endif /* TEST_GTY_H */
