/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree field2;  /* Use dummy GCC type */
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
   TYPE_POINTER: Pointer declaration
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_length;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        tree t;
    } u;
    rtx insn;  /* Use dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("strlen(%h.my_string) + 1"))) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, tree);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure traversal
   ============================================ */

/* Nested struct with multiple GTY attributes */
struct GTY((tag("outer_struct"))) outer_struct {
    struct my_struct GTY((tag("inner"))) inner;
    union my_union data;
    my_callback_fn callback;
};

/* Array of pointers */
struct my_struct* GTY((length("ptr_count"))) ptr_array[5];
extern int ptr_count;

/* Struct with callback field */
struct GTY((tag("with_callback"))) struct_with_callback {
    int id;
    my_callback_fn GTY((skip)) handler;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    struct outer_struct o;
    my_scalar_t scalar;
};

/* Forward declaration for mutual reference */
struct GTY(()) forward_declared;

/* Struct with forward reference */
struct GTY((tag("with_forward_ref"))) with_forward_ref {
    int data;
    struct forward_declared* GTY((skip)) next;
};

/* Definition of forward declared struct */
struct GTY((tag("forward_defined"))) forward_declared {
    int value;
    struct with_forward_ref* prev;
};

#endif /* TEST_STATE_GTY_H */
