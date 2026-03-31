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
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree field2;  /* Using dummy GCC type */
    struct my_undefined_struct *next;  /* Pointer to undefined type */
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
    char * GTY((skip)) b;  /* Skip this pointer */
    struct my_struct *c;
    double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Various pointer declarations
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union * GTY(()) another_pointer;   /* Another pointer */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct * GTY((length("struct_count"))) struct_array[5];

/* Helper variable for array length */
extern int my_array_length;
extern int struct_count;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void * GTY((skip)) p;
        tree t;
    } u;
    rtx code;  /* Dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef unsigned long GTY((user)) my_ulong_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("strlen(my_string)+1"))) my_string;  /* TYPE_STRING */
char * GTY((length("custom_length"))) another_string;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, void*);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);

/* ============================================
   Complex nested example to ensure traversal
   ============================================ */
struct GTY((tag("complex_struct"))) complex_type {
    my_scalar_t scalar_field;          /* User scalar */
    my_callback_fn callback;           /* Callback */
    const char * GTY((length("name_len"))) name;  /* String */
    struct my_struct * GTY((skip)) skipped_ptr;   /* Pointer */
    union my_union data;               /* Union */
    int GTY((length("data_count"))) data_array[20]; /* Array */
    struct my_lang_struct *lang_ptr;   /* Lang struct pointer */
};

/* Global variables for testing */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct complex_type GTY(()) global_complex;

/* Function prototypes */
void GTY((user)) register_callback(my_callback_fn fn);
struct my_struct * GTY(()) create_struct(void);

#endif /* TEST_STATE_GTY_H */
