/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid parsing errors */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR: Scalar type with user annotation
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
    int field1;
    my_scalar_t field2;
    struct my_undefined_struct* next;  /* Reference to undefined type */
};

/* ============================================
   TYPE_USER_STRUCT: Typedef of struct with user annotation
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag and skip annotation
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char* GTY((skip)) b;  /* Skip annotation on pointer */
    struct my_struct* c;
};

/* ============================================
   TYPE_POINTER: Pointer with skip annotation
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length annotation
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */

/* Helper variable for array length */
extern int my_array_length;

/* ============================================
   TYPE_STRING: String pointer with length annotation
   ============================================ */
const char* GTY((length("my_string_length"))) my_string;  /* TYPE_STRING */

/* Helper variable for string length */
extern int my_string_length;

/* ============================================
   TYPE_CALLBACK: Function pointer with user annotation
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {  /* TYPE_LANG_STRUCT */
    int lang_specific;
    union {
        int a;
        void* p;
        struct my_struct* s;
    } u;
    tree dummy_tree;  /* Use dummy GCC type */
    rtx dummy_rtx;    /* Use dummy GCC type */
};

/* ============================================
   Additional complex types to ensure thorough traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct* GTY((skip)) ptr1;
    union my_union data;
    struct nested_struct* next;
};

/* Array of pointers */
struct my_struct* GTY((length("ptr_array_len"))) ptr_array[5];
extern int ptr_array_len;

/* Struct containing callback */
struct GTY((tag("with_callback"))) struct_with_callback {
    my_callback_fn callback;
    int data;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    struct nested_struct n;
    my_callback_fn fn;
};

/* Forward declaration that will be TYPE_NONE in the switch */
struct GTY(()) forward_declared_struct;

/* Actual definition elsewhere - but gengtype will see this as TYPE_UNDEFINED */
struct forward_declared_struct {
    int x;
};

#endif /* TEST_STATE_GTY_H */
