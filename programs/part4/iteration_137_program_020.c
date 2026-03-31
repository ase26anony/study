/* test_structures.h - Diverse type definitions for gengtype coverage testing */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ptr;
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int int_val;
    float float_val;
    void *ptr_val;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_struct_ptr;
typedef union my_union * GTY(()) my_union_ptr;

/* TYPE_ARRAY: Array type definitions */
typedef int my_int_array[10];
typedef struct my_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    void *lang_data;
};
#endif

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        my_string_type name;
    } inner;
    
    my_struct_ptr ptr_field;
    my_int_array array_field;
    callback_func callback_field;
};

/* Another union with GTY */
union GTY(()) complex_union {
    struct my_struct s;
    struct outer_struct o;
    callback_func f;
};

#endif /* TEST_STRUCTURES_H */
