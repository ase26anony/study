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

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
struct GTY((user)) user_defined_struct {
    scalar_int_t data;
    void (* GTY((skip)) custom_func)(void);
};

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
typedef union data_union GTY(()) union_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(scalar_int_t);
typedef int (* GTY(()) filter_t)(const struct simple_struct *);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((desc("%1"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    scalar_int_t tag;
    union data_union value;
    struct lang_struct * GTY((skip)) next;
    struct lang_struct * GTY((skip)) prev;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container_struct {
    int_array_t numbers;
    struct_ptr_t structs[4];
    callback_t handlers[2];
    union GTY(()) {
        scalar_int_t mode;
        string_t mode_name;
    } config;
    struct GTY(()) {
        scalar_int_t depth;
        struct container_struct * GTY((skip)) parent;
    } metadata;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
