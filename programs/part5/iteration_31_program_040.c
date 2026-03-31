#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int GTY(());
typedef float scalar_float GTY(());
typedef double scalar_double GTY(());
typedef char scalar_char GTY(());
typedef long scalar_long GTY(());
typedef unsigned int scalar_uint GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *string_type GTY(());

/* TYPE_STRUCT: Regular struct types */
struct simple_struct GTY(()) {
    scalar_int field1;
    scalar_float field2;
    string_type field3;
};

struct nested_struct GTY(()) {
    struct simple_struct inner;
    scalar_double extra;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct simple_struct user_struct_t GTY(());

/* TYPE_UNION: Union type */
union data_union GTY(()) {
    scalar_int as_int;
    scalar_float as_float;
    string_type as_string;
    struct simple_struct *as_struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr GTY(());
typedef struct simple_struct *struct_ptr GTY(());
typedef union data_union *union_ptr GTY(());
typedef void *void_ptr GTY(());

/* TYPE_ARRAY: Array types */
typedef int int_array[10] GTY(());
typedef struct simple_struct struct_array[5] GTY(());
typedef string_type string_array[3] GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int) GTY(());
typedef int (*complex_callback)(struct simple_struct *, string_type) GTY(());
typedef void (*void_callback)(void) GTY(());

/* TYPE_LANG_STRUCT: Language-specific struct with GTY markers */
struct GTY((desc("%1.kind"), tag("lang_node"))) lang_struct {
    int kind;
    union GTY((desc("%1.kind"))) {
        struct simple_struct simple;
        struct nested_struct nested;
        string_type str;
    } GTY((tag("kind"))) u;
};

/* Complex nested type to ensure traversal */
struct container_struct GTY(()) {
    /* Contains multiple type categories */
    scalar_int count;                     /* SCALAR */
    string_type name;                     /* STRING */
    struct simple_struct data;            /* STRUCT */
    union data_union variant;             /* UNION */
    struct nested_struct *nested_ptr;     /* POINTER */
    int_array numbers;                    /* ARRAY */
    simple_callback handler;              /* CALLBACK */
    struct lang_struct lang_data;         /* LANG_STRUCT */
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
