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
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree field2;  /* Using dummy GCC type */
    struct my_undefined_struct* next;  /* Pointer to undefined type */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {
    int a;
    char* GTY((skip)) b;  /* Skip this pointer field */
    struct my_struct* c;
    double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer declaration with skip
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_length;  /* Length variable for the array */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        tree t;  /* GCC internal type */
    } u;
    rtx insn;  /* Another GCC internal type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("my_string_len"))) my_string;  /* TYPE_STRING */
extern int my_string_len;  /* Length variable */

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, tree);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("outer_struct"))) outer_struct {
    struct my_struct inner;
    union my_union* GTY((skip)) union_ptr;
    my_callback_fn callback;
};

/* Array of pointers */
struct my_struct* GTY((length("ptr_array_len"))) ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
extern int ptr_array_len;

/* Struct containing array */
struct GTY((tag("with_array"))) struct_with_array {
    int GTY((length("arr_len"))) arr[20];  /* TYPE_ARRAY inside TYPE_STRUCT */
    int arr_len;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    struct outer_struct* GTY((skip)) o;
    my_scalar_t scalar;
};

#endif /* TEST_STATE_GTY_H */
