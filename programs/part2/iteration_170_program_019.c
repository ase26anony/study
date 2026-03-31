/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

#ifdef GENERATOR_FILE
/* This macro indicates we're being processed by gengtype */
#endif

/* Include gtype-desc.h for GTY macro definition */
#include "gtype-desc.h"

/* ========== TYPE_SCALAR ========== */
/* Basic scalar typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;

/* ========== TYPE_STRING ========== */
/* String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* ========== TYPE_STRUCT ========== */
/* Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

struct another_plain {
    char data[32];
    struct plain_struct *link;
};

/* ========== TYPE_USER_STRUCT ========== */
/* GTY-tagged structs for garbage collection */

/* Simple GTY struct */
struct GTY(()) user_struct {
    int id;
    string_t name;
};

/* GTY struct with pointer to another GTY struct */
struct GTY(()) linked_struct {
    int value;
    struct user_struct *GTY((skip)) next;
    struct linked_struct *GTY((skip)) prev;
};

/* GTY struct with nested structures */
struct GTY(()) complex_struct {
    struct user_struct base;
    struct linked_struct *GTY((skip)) chain;
    void *GTY((skip)) user_data;
};

/* ========== TYPE_UNION ========== */
/* Union types */
union data_union {
    int int_val;
    double double_val;
    void *ptr_val;
    char str_val[16];
};

/* GTY-tagged union */
union GTY(()) tagged_union {
    struct user_struct *GTY((skip)) us_ptr;
    struct linked_struct *GTY((skip)) ls_ptr;
    int scalar_val;
};

/* ========== TYPE_POINTER ========== */
/* Pointer typedefs */
typedef struct user_struct *user_ptr_t;
typedef struct linked_struct **double_ptr_t;

/* ========== TYPE_ARRAY ========== */
/* Arrays within structs */
struct GTY(()) array_container {
    int fixed_array[10];
    char string_array[5][32];
    struct user_struct *GTY((skip)) ptr_array[8];
};

/* Variable length array (in GTY struct) */
struct GTY(()) vla_container {
    int count;
    int data[];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types (callbacks) */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct user_struct *, string_t);

/* GTY struct with callback field */
struct GTY(()) callback_container {
    simple_callback cb1;
    complex_callback cb2;
    void (*inline_cb)(void);
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs */
#ifdef GENERATOR_FILE
/* This struct should only be seen by gengtype */
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *GTY((skip)) gen_ptr;
};
#endif

/* Conditional struct for different contexts */
#if defined(GENERATOR_FILE) || defined(IN_GCC)
struct GTY(()) conditional_struct {
    int context_specific;
#ifdef GENERATOR_FILE
    string_t gen_string;
#else
    void *other_ptr;
#endif
};
#endif

/* ========== Complex Nested Types ========== */
/* Recursive structure */
struct GTY(()) tree_node {
    int type;
    string_t name;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union tagged_union data;
};

/* Struct with multiple array types */
struct GTY(()) multi_array_struct {
    int matrix[3][3];
    char *string_table[5];
    struct user_struct *object_grid[2][2];
    callback_container callbacks[4];
};

/* Union containing GTY pointers */
union GTY(()) gty_pointer_union {
    struct user_struct *GTY((skip)) us;
    struct linked_struct *GTY((skip)) ls;
    struct tree_node *GTY((skip)) tn;
    void *generic;
};

/* ========== Mixed Type Container ========== */
/* Struct containing examples of all type kinds */
struct GTY(()) type_showcase {
    /* SCALAR */
    my_int scalar_field;
    
    /* STRING */
    string_t string_field;
    
    /* STRUCT (embedded) */
    struct plain_struct embedded_struct;
    
    /* USER_STRUCT (pointer) */
    struct user_struct *GTY((skip)) user_struct_ptr;
    
    /* UNION */
    union data_union data_field;
    
    /* POINTER */
    void *raw_pointer;
    
    /* ARRAY */
    int number_array[5];
    
    /* CALLBACK */
    simple_callback callback_field;
    
    /* LANG_STRUCT (conditional) */
#ifdef GENERATOR_FILE
    struct lang_specific_struct *GTY((skip)) lang_struct_ptr;
#endif
    
    /* Nested showcase */
    struct type_showcase *GTY((skip)) next;
};

/* ========== Additional Pointer Types ========== */
/* Pointer to array */
typedef int (*array_ptr_t)[10];

/* Pointer to function returning pointer */
typedef struct user_struct *(*factory_fn)(int);

/* Complex function pointer */
typedef void (*event_handler)(
    struct type_showcase *GTY((skip)),
    string_t,
    simple_callback
);

#endif /* TEST_GENGTYPE_TYPES_H */
