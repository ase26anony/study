/* test_gty.h - Comprehensive test of GTY annotations for gengtype coverage */

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
   TYPE_UNDEFINED: Forward declared struct
   ============================================ */
struct GTY(()) undefined_struct;
/* This creates TYPE_UNDEFINED since it's declared but not defined */

/* ============================================
   TYPE_SCALAR: Basic scalar type with user attribute
   ============================================ */
typedef int GTY((user)) my_scalar_t;

/* ============================================
   TYPE_STRING: String with length attribute
   ============================================ */
const char * GTY((length(strlen(my_string)))) my_string;

/* ============================================
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct_tag"))) my_struct {
    int field1;
    tree field2;  /* Using dummy GCC type */
    struct undefined_struct * GTY((skip)) ptr_field;  /* Pointer to undefined */
};

/* ============================================
   TYPE_USER_STRUCT: Typedef of struct with user attribute
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ============================================
   TYPE_UNION: Union with desc attribute for discrimination
   ============================================ */
union GTY((desc("%0.type"))) my_union {
    int type;
    int GTY((tag("0"))) a;
    char * GTY((tag("1"), skip)) b;
    struct my_struct GTY((tag("2"))) c;
};

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((user)) tree_pointer;
rtx GTY((user)) rtx_pointer;

/* ============================================
   TYPE_ARRAY: Arrays with length attributes
   ============================================ */
int GTY((length("10"))) my_array[10];
struct my_struct GTY((length("5"))) struct_array[5];
const char * GTY((length("20"))) string_array[20];

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, tree);
typedef tree (*GTY((user)) tree_callback_fn)(rtx, int);

/* ============================================
   TYPE_LANG_STRUCT: Language-specific structure
   ============================================ */
/* Create a struct that mimics GCC's language-specific structures */
struct GTY((special("lang_struct"), chain_next("%h.next"), chain_prev("%h.prev"))) lang_specific {
    int lang_code;
    union my_union GTY((desc("%h.lang_code"))) data;
    struct lang_specific *next;
    struct lang_specific *prev;
};

/* ============================================
   Additional complex types to ensure traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY(()) complex_struct {
    struct my_struct base;
    union my_union variant;
    my_callback_fn callback;
    struct complex_struct * GTY((skip)) next;
    struct lang_specific *lang_data;
};

/* Array of pointers */
struct my_struct * GTY((length("8"))) pointer_array[8];

/* Union containing pointers */
union GTY((desc("%0.which"))) pointer_union {
    int which;
    struct my_struct * GTY((tag("0"))) struct_ptr;
    tree GTY((tag("1"))) tree_val;
    rtx GTY((tag("2"))) rtx_val;
};

/* Struct with embedded array */
struct GTY(()) with_array {
    int count;
    int GTY((length("%h.count"))) values[];
};

/* ============================================
   Variable declarations using the types
   ============================================ */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct lang_specific * GTY((user)) lang_struct_ptr;
extern my_callback_fn GTY((user)) global_callback;

#endif /* TEST_GTY_H */
