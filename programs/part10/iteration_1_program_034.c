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
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree field2;  /* Use dummy GCC type */
    struct my_undefined_struct* next;  /* Reference undefined type */
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
union my_union* GTY(()) another_pointer;   /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("10"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct* GTY((length("5"))) struct_array[5];  /* TYPE_ARRAY of pointers */

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
    rtx code;
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef double GTY((user)) my_double_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("strlen(%h.my_string)"))) my_string;  /* TYPE_STRING */
char* GTY((length("10"))) fixed_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, tree);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) another_callback)(void*);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure full traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct* GTY((skip)) first;
    union my_union data;
    my_callback_fn callback;
    int GTY((length("data_count"))) dynamic_array[];
};

/* Container struct referencing all types */
struct GTY((tag("container"))) type_container {
    /* TYPE_STRUCT fields */
    struct my_struct regular_struct;
    
    /* TYPE_USER_STRUCT */
    my_user_struct_t user_struct;
    
    /* TYPE_UNION */
    union myunion_data;
    
    /* TYPE_POINTER */
    struct nested_struct* GTY((skip)) nested_ptr;
    
    /* TYPE_ARRAY */
    my_scalar_t GTY((length("5"))) scalar_array[5];
    
    /* TYPE_LANG_STRUCT */
    struct my_lang_struct lang_data;
    
    /* TYPE_SCALAR */
    my_scalar_t scalar_field;
    
    /* TYPE_STRING */
    const char* GTY((length("strlen(%h.name)"))) name;
    
    /* TYPE_CALLBACK */
    my_callback_fn handler;
    
    /* Reference to undefined type */
    struct my_undefined_struct* GTY((skip)) undefined_ref;
};

#endif /* TEST_STATE_GTY_H */
