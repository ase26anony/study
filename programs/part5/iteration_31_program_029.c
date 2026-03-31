#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;

/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Regular structures */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct * GTY(()) nested;
};

/* TYPE_USER_STRUCT: User-defined structure type */
typedef struct GTY(()) user_def {
    int data;
    void * GTY(()) extra;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) processor_t)(const char *, void *);

/* TYPE_LANG_STRUCT: Language-specific structure with GC roots */
struct GTY((user)) lang_specific_struct {
    int tag;
    union {
        struct GTY((desc("%0.tag"))) lang_specific_struct *left;
        int value;
    } GTY((tag("0"))) u;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains all type categories */
    scalar_int_t scalar_field;          /* TYPE_SCALAR */
    string_t string_field;              /* TYPE_STRING */
    struct simple_struct struct_field;  /* TYPE_STRUCT */
    union data_union union_field;       /* TYPE_UNION */
    int_ptr_t pointer_field;            /* TYPE_POINTER */
    int_array_t array_field;            /* TYPE_ARRAY */
    callback_t callback_field;          /* TYPE_CALLBACK */
    struct lang_specific_struct * GTY(()) lang_field; /* TYPE_LANG_STRUCT */
    
    /* Self-referential pointer */
    struct container_struct * GTY(()) next;
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
