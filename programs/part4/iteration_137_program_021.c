/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) regular_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *undef_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Structures with GTY((user)) */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* Another regular struct for more coverage */
struct GTY(()) another_struct {
    my_string_type name;
    int value;
};

/* TYPE_UNION: Union with GTY(()) */
union GTY(()) my_union {
    int int_val;
    double double_val;
    my_string_type str_val;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef struct regular_struct *regular_struct_ptr;
typedef my_scalar *scalar_ptr;
typedef const int *const_int_ptr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct regular_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(struct regular_struct*, my_scalar);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structure for complex testing */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        my_union inner_union;
    } inner;
    
    regular_struct_ptr ptr_field;
    int_array array_field;
    callback_type callback_field;
};

/* Global variable to force gengtype processing */
extern struct regular_struct GTY(()) global_struct_var;
extern my_string_type GTY(()) global_string_var;
extern union my_union GTY(()) global_union_var;

#endif /* TEST_STRUCTURES_H */
