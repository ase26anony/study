#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
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
struct GTY(()) regular_struct {
    scalar_int_t id;
    scalar_float_t value;
};

struct GTY(()) another_struct {
    string_t name;
    scalar_double_t data;
};

/* TYPE_USER_STRUCT: Struct with user-defined alignment/size */
struct GTY((user)) user_struct {
    scalar_int_t * GTY((skip)) user_data;
    scalar_char_t flags;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef scalar_int_t * GTY(()) int_ptr_t;
typedef struct regular_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef scalar_int_t GTY(()) int_array_t[10];
typedef struct regular_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(scalar_int_t);
typedef scalar_int_t (* GTY(()) compute_t)(scalar_float_t, scalar_double_t);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((desc("%1.lang_code"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int lang_code;
    string_t lang_name;
    struct lang_struct *next;
    struct lang_struct *prev;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t count;                     /* SCALAR */
    string_t description;                   /* STRING */
    struct regular_struct base;             /* STRUCT */
    union data_union variant;               /* UNION */
    int_ptr_t *pointer_array;               /* POINTER (in array) */
    int_array_t numbers;                    /* ARRAY */
    callback_t handler;                     /* CALLBACK */
    struct lang_struct *lang_info;          /* LANG_STRUCT */
    struct undefined_struct *future;        /* UNDEFINED */
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
