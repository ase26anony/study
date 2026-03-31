/* test_gty.h - Comprehensive GTY-annotated types for gengtype coverage */
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

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) undefined_struct;  /* Expected: TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR: Basic scalar type with user annotation
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* Expected: TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length attribute
   ============================================ */
const char * GTY((length("strlen(%h.my_string)"))) my_string;  /* Expected: TYPE_STRING */

/* ============================================
   TYPE_STRUCT: Regular struct with tag attribute
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* Expected: TYPE_STRUCT */
    int field;
    my_scalar_t scalar_field;
};

/* ============================================
   TYPE_USER_STRUCT: Typedef of struct with user annotation
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* Expected: TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc attribute for discriminant
   ============================================ */
union GTY((desc("$a ? 1 : 0"))) my_union {  /* Expected: TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* ============================================
   TYPE_POINTER: Pointer to struct with skip attribute
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* Expected: TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("sizeof(%h.my_array)/sizeof(%h.my_array[0])"))) my_array[10];  /* Expected: TYPE_ARRAY */

/* ============================================
   TYPE_CALLBACK: Function pointer with user annotation
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* Expected: TYPE_CALLBACK */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific structure pattern
   ============================================ */
/* Mimic a language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {  /* Expected: TYPE_LANG_STRUCT */
    int lang_code;
    union {
        tree tree_node;
        rtx rtx_value;
    } GTY((desc("%0.lang_code"))) u;
};

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY(()) complex_struct {
    struct my_struct *first;
    struct GTY((tag("nested"))) nested {
        int data;
        struct nested *next;
    } *nested_ptr;
    union my_union choice;
};

/* Array of pointers */
struct my_struct * GTY((length("5"))) ptr_array[5];

/* Pointer to array */
int (*GTY((user)) ptr_to_array)[10];

/* Struct containing callback */
struct GTY(()) with_callback {
    my_callback_fn callback;
    int data;
};

/* Forward declaration that will be defined later */
struct GTY(()) later_defined;
struct GTY(()) later_defined {
    int value;
};

#endif /* TEST_GTY_H */
