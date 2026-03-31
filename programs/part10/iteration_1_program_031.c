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
extern int my_array_length;  /* Length variable */

/* Another array example with pointer elements */
struct my_struct* GTY((length("lang_struct_count"))) lang_struct_array[5];

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

extern int lang_struct_count;  /* For array length */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* Another scalar example */
typedef unsigned long GTY((user)) my_ulong_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("string_length"))) my_string;  /* TYPE_STRING */
extern int string_length;

/* Another string example */
char* GTY((length("name_len"))) name_ptr;
extern int name_len;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, const char*);  /* TYPE_CALLBACK */

/* Another callback example with struct parameter */
typedef int (*GTY((user)) compare_fn)(struct my_struct*, struct my_struct*);

/* ============================================
   Complex nested example to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
    my_scalar_t scalar_field;          /* User scalar */
    my_callback_fn callback_field;     /* Callback */
    union my_union union_field;        /* Union */
    struct my_struct* GTY((skip)) ptr_field;  /* Skip pointer */
    const char* GTY((length("str_len"))) str_field;  /* String */
    int GTY((length("array_len"))) int_array[20];  /* Array */
    struct my_lang_struct lang_field;  /* Lang struct */
};

extern int str_len;
extern int array_len;

/* ============================================
   Container struct to hold multiple types
   ============================================ */
struct GTY((tag("container"))) type_container {
    /* Undefined type pointer */
    struct my_undefined_struct* GTY((skip)) undefined_ptr;
    
    /* Regular struct */
    struct my_struct regular_struct;
    
    /* User struct */
    my_user_struct_t user_struct;
    
    /* Union */
    union my_union data_union;
    
    /* Pointer */
    struct my_struct* direct_pointer;
    
    /* Array */
    int GTY((length("cont_array_len"))) cont_array[15];
    
    /* Lang struct */
    struct my_lang_struct lang_item;
    
    /* Scalar */
    my_scalar_t scalar_item;
    
    /* String */
    const char* GTY((length("cont_str_len"))) cont_string;
    
    /* Callback */
    my_callback_fn callback_item;
};

extern int cont_array_len;
extern int cont_str_len;

#endif /* TEST_STATE_GTY_H */
