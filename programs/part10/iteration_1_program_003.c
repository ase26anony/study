/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid dependencies */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

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
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char* GTY((skip)) b;  /* Skip annotation on pointer field */
    struct my_struct* c;
};

/* ============================================
   TYPE_POINTER: Pointer declaration with skip
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("10"))) my_array[10];  /* TYPE_ARRAY */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length)) my_string;  /* TYPE_STRING */

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
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct* GTY((skip)) ptr1;
    union my_union data;
    int GTY((length("5"))) small_array[5];
};

/* Array of pointers */
struct my_struct* GTY((length("3"))) ptr_array[3];

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct {
        int x;
        int y;
    } GTY((tag("point"))) point;
    struct my_lang_struct* lang_ptr;
};

/* Struct with callback field */
struct GTY((tag("with_callback"))) struct_with_callback {
    int id;
    my_callback_fn GTY((skip)) callback;
    const char* GTY((length)) name;
};

/* Forward declaration that will be TYPE_UNDEFINED */
union GTY(()) forward_union;

/* Complete the forward declaration */
union GTY((desc("2"))) forward_union {
    int complete_field;
    struct my_struct* ptr_field;
};

#endif /* TEST_STATE_GTY_H */
