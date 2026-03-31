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
typedef const char *GTY(()) string_t;

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct *GTY((skip)) nested;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct GTY(()) user_def_struct {
    int data;
    void *GTY((skip)) opaque;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef int *GTY(()) int_ptr_t;
typedef struct simple_struct *GTY(()) struct_ptr_t;
typedef union data_union *GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (*GTY(()) callback_t)(int);
typedef int (*GTY(()) processor_t)(const char *, void *);

/* TYPE_LANG_STRUCT: Language-specific struct with GC annotations */
struct GTY((desc("%0.type"), tag("TREE_TYPE"))) lang_struct {
    int type;
    union {
        struct simple_struct *GTY((tag("0"))) simple;
        struct complex_struct *GTY((tag("1"))) complex;
    } GTY((desc("%1.type"))) u;
};

/* Nested types to ensure graph traversal */
struct GTY(()) container_struct {
    /* Contains array of pointers */
    struct_ptr_t GTY((length("%0.count"))) *items;
    int count;
    
    /* Contains union */
    union data_union value;
    
    /* Contains callback */
    callback_t handler;
    
    /* Contains nested struct */
    struct {
        int_array_t numbers;
        string_t label;
    } GTY(()) metadata;
};

/* Include auxiliary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
