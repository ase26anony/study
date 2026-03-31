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
    tree field2;  /* Use dummy GCC type */
    void* GTY((skip)) skip_field;  /* Skip this pointer */
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
    char* GTY((skip)) b;
    struct my_struct* GTY((skip)) c;
    double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer type
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_len;  /* Length variable */

/* Another array example with nested struct */
struct my_struct GTY((length("nested_len"))) nested_array[5];  /* TYPE_ARRAY */
extern int nested_len;

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
    rtx rtx_field;  /* Use dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* Alternative lang struct using chain_next */
struct GTY((chain_next("next"))) chain_struct {
    int value;
    struct chain_struct* next;
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar type with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef double GTY((user)) my_double_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("str_len"))) my_string;  /* TYPE_STRING */
extern int str_len;

/* Another string example */
char* GTY((length("dynamic_len"))) dynamic_string;  /* TYPE_STRING */
extern int dynamic_len;

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, char*);  /* TYPE_CALLBACK */

/* Another callback with struct parameter */
typedef int (*GTY((user)) process_struct_fn)(struct my_struct*);  /* TYPE_CALLBACK */

/* ============================================
   Complex nested example to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
    /* Mix of different field types */
    int scalar_field;
    struct my_struct* GTY((skip)) struct_ptr;
    union my_union data;
    int GTY((length("array_field_len"))) array_field[20];
    const char* GTY((length("name_len"))) name;
    void (*GTY((user)) handler)(void);
};

/* Global variables to satisfy length references */
int my_array_len = 10;
int nested_len = 5;
int str_len = 0;
int dynamic_len = 0;
int array_field_len = 20;
int name_len = 0;

/* ============================================
   Additional undefined type for completeness
   ============================================ */
union GTY(()) another_undefined_union;  /* TYPE_UNDEFINED */

#endif /* TEST_STATE_GTY_H */
