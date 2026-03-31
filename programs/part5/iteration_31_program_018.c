#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR: Basic scalar types with GTY annotations */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* TYPE_STRING: String pointer typedef */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Various struct definitions */
struct GTY(()) simple_struct {
    scalar_int_t id;
    string_t name;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    scalar_float_t values[5];
    struct simple_struct * GTY((skip)) next;
};

/* TYPE_USER_STRUCT: Struct with user-defined base type */
typedef struct simple_struct user_struct_base_t;
struct GTY((user)) user_struct {
    user_struct_base_t base;
    scalar_double_t extra_data;
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
typedef int (* GTY(())) process_func_t)(const char *, int);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct {
    int lang_specific_data;
    struct lang_struct *next;
    struct lang_struct *prev;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t scalar_field;          /* TYPE_SCALAR */
    string_t string_field;              /* TYPE_STRING */
    struct simple_struct struct_field;  /* TYPE_STRUCT */
    union data_union union_field;       /* TYPE_UNION */
    int_ptr_t pointer_field;            /* TYPE_POINTER */
    int_array_t array_field;            /* TYPE_ARRAY */
    callback_t callback_field;          /* TYPE_CALLBACK */
    struct lang_struct *lang_field;     /* TYPE_LANG_STRUCT */
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
