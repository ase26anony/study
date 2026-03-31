/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype headers if available */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Forward declarations to create dependencies */
struct forward_declared_s;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_int;
typedef unsigned long my_ulong;
typedef double my_double;
typedef char my_char;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Union definitions */
union plain_union {
    int i;
    float f;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, void *);

/* TYPE_ARRAY: Array typedef */
typedef int int_array[10];

/* GTY-tagged types section - these will be processed by gengtype */

/* TYPE_USER_STRUCT: GTY-tagged struct */
struct GTY(()) user_struct {
    /* TYPE_POINTER: Pointer field */
    struct user_struct *next;
    
    /* TYPE_SCALAR in GTY context */
    int id;
    
    /* TYPE_STRING in GTY context */
    const char *name;
    
    /* TYPE_ARRAY in GTY context */
    int values[5];
    
    /* TYPE_POINTER to plain struct */
    struct plain_struct *plain_ptr;
    
    /* TYPE_CALLBACK in GTY context */
    simple_callback cb;
};

/* Another GTY-tagged struct for complex dependencies */
struct GTY(()) complex_struct {
    /* Nested GTY struct */
    struct user_struct *user;
    
    /* Array of pointers */
    struct user_struct *GTY((length("count"))) *items;
    int count;
    
    /* Pointer to forward declared struct */
    struct forward_declared_s *forward_ptr;
    
    /* Union containing GTY pointer */
    union {
        struct user_struct *gty_ptr;
        void *raw_ptr;
    } GTY((tag("type"))) u;
    int type;
};

/* Forward declared struct definition */
struct GTY(()) forward_declared_s {
    int magic;
    struct complex_struct *parent;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int generator_only_field;
    struct user_struct *gty_field;
};
#endif

/* Another language-specific variant */
#ifdef LANG_HOOKS
struct GTY(()) another_lang_struct {
    void *lang_data;
    int lang_id;
};
#endif

/* Union with GTY marker */
union GTY(()) gty_union {
    struct user_struct *as_user;
    struct complex_struct *as_complex;
    long as_long;
};

/* Struct containing array of structs */
struct GTY(()) array_container {
    /* Array of structs */
    struct user_struct GTY((length("arr_count"))) arr[10];
    int arr_count;
    
    /* Pointer to array */
    int *GTY((length("ptr_count"))) ptr_arr;
    int ptr_count;
};

/* Callback in GTY struct */
struct GTY(()) callback_container {
    /* TYPE_CALLBACK field */
    complex_callback handler;
    
    /* Nested callback type */
    void (*GTY((skip)) nested_cb)(struct callback_container *);
    
    void *user_data;
};

/* Root structure for GC */
struct GTY(()) root_struct {
    struct user_struct *first;
    struct complex_struct *complex;
    struct array_container *array;
    union gty_union uni;
    struct callback_container *callbacks[5];
};

/* Additional scalar typedefs that might be referenced */
typedef struct user_struct *user_ptr_t;
typedef union gty_union gty_union_t;

#endif /* TEST_GENGTYPE_TYPES_H */
