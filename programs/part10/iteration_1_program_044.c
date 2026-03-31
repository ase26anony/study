/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (as in standalone gengtype) */
#ifndef GTY
#define GTY(x) __attribute__((garbage_collected(x)))
#endif

/* Dummy definitions for GCC internal types to avoid dependency issues */
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
   TYPE_POINTER: Various pointer declarations
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */
union my_union* GTY(()) my_union_ptr;      /* Another pointer */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */
struct my_struct* GTY((length("5"))) struct_array[5];

/* Variable to hold array length (referenced in length attribute) */
extern int my_array_length;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        tree t;  /* GCC internal type */
    } u;
    rtx r;  /* Another GCC internal type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */
typedef double GTY((user)) my_double_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("strlen(%h.my_string) + 1"))) my_string;  /* TYPE_STRING */
char* GTY((length("10"))) fixed_string;

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */
typedef int (*GTY((user)) another_callback)(tree, rtx);

/* ============================================
   Nested structures to ensure deep traversal
   ============================================ */
struct GTY(()) container_struct {
    struct my_struct embedded;      /* Embedded struct */
    union my_union GTY((tag("1"))) u_embed;  /* Embedded union with tag */
    my_scalar_t scalar_field;       /* User scalar type */
    my_callback_fn callback;        /* Callback field */
    
    /* Pointer chain */
    struct container_struct* GTY((skip)) next;
    struct container_struct* GTY(()) prev;
};

/* ============================================
   Global variables with GTY markers
   ============================================ */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern my_scalar_t GTY(()) global_scalar;

/* ============================================
   More complex cases for thorough testing
   ============================================ */
struct GTY(()) complex_type {
    /* Array of pointers */
    struct my_struct* GTY((length("array_len"))) ptr_array[8];
    
    /* Union containing different types */
    union {
        int GTY((tag("0"))) as_int;
        struct my_struct* GTY((tag("1"))) as_struct;
        const char* GTY((tag("2"))) as_string;
    } GTY((desc("%0.type"))) variant;
    
    int type;  /* For descriminator */
    int array_len;  /* For array length */
};

#endif /* TEST_STATE_GTY_H */
