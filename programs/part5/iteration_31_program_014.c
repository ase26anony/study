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

/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    scalar_int_t id;
    string_t name;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    scalar_double_t value;
    struct simple_struct * GTY(()) nested;
};

/* TYPE_USER_STRUCT: Struct with user-defined alignment/size */
struct GTY((user)) user_struct {
    scalar_int_t data;
    scalar_char_t flags[4];
};

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
typedef int (* GTY(()) filter_t)(const char *, int);
typedef void (* GTY(()) complex_callback_t)(struct simple_struct *, union data_union);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((desc("%1"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int GTY(()) lang_specific_data;
    struct lang_struct * GTY(()) next;
    struct lang_struct * GTY(()) prev;
    callback_t GTY(()) handler;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t GTY(()) id;                    /* SCALAR */
    string_t GTY(()) description;               /* STRING */
    struct simple_struct GTY(()) base;          /* STRUCT */
    union data_union GTY(()) data;              /* UNION */
    int_ptr_t GTY(()) int_pointer;              /* POINTER */
    int_array_t GTY(()) numbers;                /* ARRAY */
    callback_t GTY(()) notify;                  /* CALLBACK */
    struct lang_struct * GTY(()) lang_data;     /* LANG_STRUCT */
    struct undefined_struct * GTY(()) undefined; /* Will be TYPE_UNDEFINED */
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
