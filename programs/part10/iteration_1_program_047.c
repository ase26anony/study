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
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
    int field1;
    my_scalar_t field2;
};

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* ============================================
   TYPE_POINTER: Pointer with skip attribute
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_len;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("my_strlen"))) my_string;  /* TYPE_STRING */
extern int my_strlen;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {  /* TYPE_LANG_STRUCT */
    int lang_specific;
    union {
        int a;
        void * GTY((skip)) p;
    } u;
    tree dummy_tree;  /* Use dummy GCC type */
};

/* ============================================
   Additional complex types to ensure traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct * GTY((skip)) ptr1;
    union my_union data;
    my_callback_fn callback;
};

/* Array of pointers */
struct my_struct * GTY((length("ptr_array_len"))) ptr_array[5];  /* TYPE_ARRAY */
extern int ptr_array_len;

/* Struct with multiple GTY attributes */
struct GTY((tag("complex"), chain_next("next"), chain_prev("prev"))) complex_struct {
    int id;
    const char * GTY((length("name_len"))) name;
    struct complex_struct *next;
    struct complex_struct *prev;
    rtx insn;  /* Dummy GCC type */
};
extern int name_len;

/* Union with nested struct */
union GTY((desc("1"))) nested_union {
    struct {
        int x;
        int y;
    } GTY((tag("point"))) point;
    struct {
        const char * GTY((length("text_len"))) text;
        int length;
    } GTY((tag("text_data"))) text_data;
};
extern int text_len;

/* ============================================
   Variable declarations for the types
   ============================================ */

/* Declare instances to ensure they're processed */
extern struct my_struct global_struct;
extern union my_union global_union;
extern struct nested_struct global_nested;
extern struct complex_struct * GTY((skip)) global_complex_ptr;
extern union nested_union global_nested_union;

/* Array of unions */
union my_union GTY((length("union_array_len"))) union_array[3];  /* TYPE_ARRAY */
extern int union_array_len;

/* String array */
const char * GTY((length("str_array_len"))) string_array[] = {  /* TYPE_ARRAY */
    "hello",
    "world",
    "test"
};
extern int str_array_len;

#endif /* TEST_STATE_GTY_H */
