/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (as in standalone gengtype run) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types to avoid parsing errors */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Regular struct with tag
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
   TYPE_UNION: Union with descriminator
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

/* Helper for array length */
extern int my_array_length;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        tree t;  /* GCC type */
    } u;
    rtx insn;  /* Another GCC type */
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
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct* GTY((tag("my_struct"))) first;
    union my_union data;
    my_callback_fn callback;
};

/* Array of pointers */
struct my_struct* GTY((length("5"))) ptr_array[5];

/* Struct containing array */
struct GTY((tag("with_array"))) struct_with_array {
    int GTY((length("10"))) values[10];
    int count;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    struct nested_struct ns;
    my_scalar_t scalar;
};

/* Pointer to undefined type (should still be processed) */
struct my_undefined_struct* GTY((skip)) undefined_ptr;

/* Chain of types for deep traversal */
typedef struct chain_node GTY((tag("chain_node"))) chain_node_t;

struct GTY((tag("chain_node"))) chain_node {
    int value;
    chain_node_t* GTY((skip)) next;
    chain_node_t* GTY((skip)) prev;
};

#endif /* TEST_STATE_GTY_H */
