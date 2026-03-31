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
struct GTY(()) simple_struct {
    scalar_int_t id;
    string_t name;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    scalar_double_t value;
    string_t description;
    struct simple_struct * GTY((skip)) simple; /* Skip GC for testing */
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct GTY(()) user_def_struct {
    scalar_int_t data;
    scalar_float_t precision;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef scalar_int_t * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef scalar_int_t GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(scalar_int_t);
typedef scalar_int_t (* GTY(()) compute_t)(scalar_double_t, scalar_double_t);

/* TYPE_LANG_STRUCT: Language-specific struct with GTY annotation */
struct GTY((user)) lang_specific_struct {
    scalar_int_t lang_id;
    string_t lang_name;
    callback_t lang_callback;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t GTY((skip)) id;                    /* Scalar */
    string_t GTY((skip)) name;                     /* String */
    struct simple_struct GTY((skip)) simple;       /* Struct */
    union data_union GTY((skip)) data;            /* Union */
    int_ptr_t GTY((skip)) ptr_array[5];           /* Array of pointers */
    callback_t GTY((skip)) handlers[3];           /* Array of callbacks */
    struct lang_specific_struct * GTY((skip)) lang_struct; /* Pointer to lang struct */
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
