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
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree field2;  /* Use dummy GCC type */
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
    char * GTY((skip)) b;  /* Skip this pointer field */
    struct my_struct *c;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer declaration
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_len;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void * GTY((tag("0"))) p;
    } u;
    tree base;  /* Common GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("my_strlen"))) my_string;  /* TYPE_STRING */
extern int my_strlen;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, tree);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("outer_struct"))) outer_struct {
    struct my_struct * GTY((skip)) ptr1;
    union my_union data;
    my_scalar_t scalar;
};

/* Array of pointers */
struct my_struct * GTY((length("ptr_array_len"))) ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
extern int ptr_array_len;

/* Struct containing callback */
struct GTY((tag("callback_holder"))) callback_holder {
    my_callback_fn callback;
    int id;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    struct outer_struct * GTY((skip)) o;
    rtx insn;  /* Dummy GCC type */
};

/* Forward declaration that will be TYPE_UNDEFINED */
struct GTY(()) another_undefined;

/* Typedef for undefined struct */
typedef struct another_undefined GTY((user)) undefined_user_t;

#endif /* TEST_STATE_GTY_H */
