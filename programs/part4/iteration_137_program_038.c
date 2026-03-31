/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ref;  /* Uses undefined type */
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* Another regular structure */
struct GTY(()) another_struct {
    my_struct *ptr;
    int count;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*func_ptr_type)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);

/* Nested structure to test deeper parsing */
struct GTY(()) container {
    my_struct items[10];          /* TYPE_ARRAY inside struct */
    callback_type callback;       /* TYPE_CALLBACK field */
    my_string_type name;          /* TYPE_STRING field */
    union my_union data;          /* TYPE_UNION field */
};

/* Global variable to force processing */
extern struct GTY(()) container *global_container;

#endif /* TEST_STRUCTURES_H */
