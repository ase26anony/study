/* test_state_gty.h - Comprehensive GTY annotations for all type categories */

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
   TYPE_USER_STRUCT: Typedef with user annotation
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char* GTY((skip)) b;  /* Skip pointer in union */
    struct my_struct* c;
};

/* ============================================
   TYPE_POINTER: Various pointer types
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */
struct my_undefined_struct* GTY((skip)) undefined_ptr;  /* Another pointer */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct GTY((length("struct_array_len"))) struct_array[5];

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("strlen(%h)"))) my_string;  /* TYPE_STRING */
char* GTY((length("custom_len_func"))) dynamic_string;

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
};

/* ============================================
   TYPE_CALLBACK: Function pointer with user annotation
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) compare_fn)(const void*, const void*);

/* ============================================
   Complex nested structure to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
    /* Mix of different field types */
    my_scalar_t scalar_field;
    struct my_struct* GTY((skip)) struct_ptr;
    union my_union data;
    int GTY((length("count"))) dynamic_array[];
};

/* ============================================
   Another union with nested structures
   ============================================ */
union GTY((desc("%d.type"))) tagged_union {
    int type;
    struct my_struct s;
    struct my_lang_struct ls;
    char* GTY((skip)) str;
};

/* ============================================
   Global variables with GTY annotations
   ============================================ */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct my_lang_struct* GTY((skip)) lang_struct_ptr;

#endif /* TEST_STATE_GTY_H */
