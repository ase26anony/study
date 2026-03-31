#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Regular struct types */
struct simple_struct GTY(())
{
    scalar_int_t id;
    string_t name;
};

struct complex_struct GTY(())
{
    scalar_int_t count;
    scalar_float_t values[4];
    struct simple_struct * GTY(()) next;
};

/* TYPE_USER_STRUCT: Struct with user-defined base */
typedef struct simple_struct user_struct_base_t;

struct user_struct GTY((user))
{
    user_struct_base_t base;
    scalar_double_t extra_data;
};

/* TYPE_UNION: Union type */
union data_union GTY(())
{
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void * GTY(()) as_pointer;
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
typedef void (* GTY(()) simple_callback_t)(int);
typedef int (* GTY(()) complex_callback_t)(struct simple_struct *, string_t);
typedef union data_union (* GTY(()) returning_callback_t)(int, float);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((desc("%1"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct
{
    struct lang_struct * GTY((skip)) next;
    struct lang_struct * GTY((skip)) prev;
    scalar_int_t lang_specific_data;
    complex_callback_t handler;
};

/* Nested complex type to ensure traversal */
struct container_struct GTY(())
{
    /* Contains multiple type categories */
    scalar_int_t scalar_field;
    string_t string_field;
    struct simple_struct struct_field;
    union data_union union_field;
    int_ptr_t pointer_field;
    int_array_t array_field;
    simple_callback_t callback_field;
    struct lang_struct * GTY(()) lang_struct_field;
    
    /* Array of pointers to unions */
    union data_union * GTY(()) union_ptr_array[8];
    
    /* Pointer to array of structs */
    struct simple_struct (* GTY(()) struct_array_ptr)[4];
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
