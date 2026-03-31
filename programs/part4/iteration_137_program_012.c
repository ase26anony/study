/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ref;  /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: Struct marked with GTY((user)) */
struct GTY((user)) user_struct {
    int user_data;
    void *user_pointer;
};

/* Another regular struct without GTY */
struct plain_struct {
    double x, y;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int as_int;
    float as_float;
    struct my_struct *as_struct;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef struct my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);

/* Nested structure to test deeper type traversal */
struct GTY(()) outer_struct {
    struct my_struct inner;
    union my_union data;
    callback_type callback;
    int_array numbers;
};

/* Language-specific structure for TYPE_LANG_STRUCT */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_data;
    void *lang_pointer;
};
#endif

/* More complex examples to ensure thorough traversal */

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Struct containing pointers of various types */
struct GTY(()) pointer_container {
    my_struct_ptr struct_ptr;
    int_ptr int_ptr;
    void_func_ptr func_ptr;
    array_ptr arr_ptr;
};

/* Union with GTY for coverage */
union GTY(()) tagged_union {
    int type;
    struct {
        int a, b;
    } pair;
    struct my_struct complex;
};

#endif /* TEST_STRUCTURES_H */
