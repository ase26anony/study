/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types */
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
   TYPE_POINTER: Various pointer declarations
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union* GTY(()) union_ptr;
my_user_struct_t* GTY(()) user_struct_ptr;

/* ============================================
   TYPE_ARRAY: Arrays with length attributes
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct* GTY((length("struct_array_len"))) struct_array[5];
const char* GTY((length("string_array_len"))) string_array[3];

/* Variable to hold array lengths (referenced in length attributes) */
extern int my_array_len;
extern int struct_array_len;
extern int string_array_len;

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
    rtx rtx_field;  /* GCC internal type */
};  /* TYPE_LANG_STRUCT */

/* Alternative lang struct pattern */
struct GTY(()) lang_struct_base {
    int base_field;
};

struct GTY(()) my_other_lang_struct {
    struct lang_struct_base base;
    union GTY((desc("1"))) {
        int type1;
        tree type2;
        struct my_struct* type3;
    } GTY((tag("0"))) data;
};

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef unsigned long GTY((user)) my_ulong_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("str_len"))) my_string;  /* TYPE_STRING */
char* GTY((length("dynamic_str_len"))) dynamic_string;
extern int str_len;
extern int dynamic_str_len;

/* ============================================
   TYPE_CALLBACK: Function pointer typedefs
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);
typedef tree (*GTY((user)) tree_walk_fn)(tree, void*);

/* ============================================
   Complex nested structure to ensure full traversal
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
    my_scalar_t scalar_field;
    my_user_struct_t* user_ptr;
    union my_union data_union;
    struct my_lang_struct* GTY((skip)) lang_ptr;
    my_callback_fn callback;
    int GTY((length("nested_len"))) nested_array[5];
    const char* GTY((length("desc_len"))) description;
};

extern int nested_len;
extern int desc_len;

/* ============================================
   Root variable declarations for gengtype to process
   ============================================ */
extern struct my_struct GTY(()) root_struct;
extern union my_union GTY(()) root_union;
extern struct my_lang_struct GTY(()) root_lang_struct;
extern struct complex_struct GTY(()) root_complex;
extern my_callback_fn GTY(()) root_callback;

#endif /* TEST_STATE_GTY_H */
