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
   TYPE_UNDEFINED: Forward declaration without definition
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
   TYPE_UNION: Union with descriminator
   ============================================ */
union GTY((desc("0"))) my_union {
    int a;
    char* GTY((skip)) b;  /* Skip this pointer field */
    struct my_struct* c;
    double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union* GTY(()) union_ptr;
my_user_struct_t* GTY(()) user_struct_ptr;

/* ============================================
   TYPE_ARRAY: Arrays with length attributes
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct* GTY((length("struct_count"))) struct_array[5];
const char* GTY((length("str_len"))) string_array[3];

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
    rtx insn;  /* Dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef unsigned long GTY((user)) my_ulong_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("strlen(my_string)+1"))) my_string;  /* TYPE_STRING */
char* GTY((length("custom_length_func(ptr)"))) dynamic_string;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, struct my_struct*);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);

/* ============================================
   Complex nested structure to exercise all paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
    /* Mix of all types */
    my_scalar_t scalar_field;          /* TYPE_SCALAR */
    my_user_struct_t user_field;       /* TYPE_USER_STRUCT */
    union my_union union_field;        /* TYPE_UNION */
    struct my_struct* GTY((skip)) ptr_field;  /* TYPE_POINTER */
    int GTY((length("array_len"))) array_field[8];  /* TYPE_ARRAY */
    struct my_lang_struct lang_field;  /* TYPE_LANG_STRUCT */
    const char* GTY((length("name_len"))) name;  /* TYPE_STRING */
    my_callback_fn callback;           /* TYPE_CALLBACK */
    
    /* Nested undefined reference */
    struct my_undefined_struct* GTY(()) undefined_ptr;  /* TYPE_UNDEFINED */
};

/* Global variables to ensure they're processed */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct complex_struct GTY(()) global_complex;

#endif /* TEST_STATE_GTY_H */
