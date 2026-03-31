/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) __attribute__((gty))
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;
/* Expected: nb_undefined++ */

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;
/* Expected: nb_scalar++ */

/* ========== TYPE_STRING ========== */
const char * GTY((length(strlen(my_string)))) my_string;
/* Expected: nb_string++ */

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct *GTY((skip)) next;
};
/* Expected: nb_struct++ */

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;
/* Expected: nb_user_struct++ */

/* ========== TYPE_UNION ========== */
union GTY((desc("%0.a"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};
/* Expected: nb_union++ */

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((skip)) tree_pointer;
/* Expected: nb_pointer++ */

/* ========== TYPE_ARRAY ========== */
int GTY((length("10"))) my_array[10];
struct my_struct GTY((length("5"))) struct_array[5];
/* Expected: nb_array++ */

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef int (*GTY((user)) another_callback)(tree, rtx);
/* Expected: nb_callback++ */

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"), desc("%d.kind"))) lang_specific {
    enum { LANG_A, LANG_B, LANG_C } kind;
    union {
        int GTY((tag("LANG_A"))) int_val;
        char * GTY((tag("LANG_B"))) str_val;
        double GTY((tag("LANG_C"))) dbl_val;
    } u;
};
/* Expected: nb_lang_struct++ */

/* Additional complex types to ensure thorough parsing */
struct GTY(()) complex_type {
    /* Nested pointer array */
    struct my_struct * GTY((length("3"))) ptr_array[3];
    
    /* Function pointer member */
    my_callback_fn GTY((skip)) callback;
    
    /* Union member */
    union my_union GTY((desc("%d.a"))) data;
    
    /* String pointer */
    const char * GTY((length(strlen(name)))) name;
};

/* Template-like pattern (common in GCC) */
struct GTY(()) template_struct {
    int GTY((skip)) count;
    void * GTY((length("%h.count"))) data;
};

/* Chain structure for linked list testing */
struct GTY((chain_next("%h.next"))) chain_node {
    int value;
    struct chain_node * GTY((skip)) next;
};

/* Variable-length array structure */
struct GTY(()) varray {
    int size;
    int GTY((length("%h.size"))) elements[1];
};

#endif /* TEST_GTY_H */
