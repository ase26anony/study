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

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct marked with GTY((user)) */
struct GTY((user)) user_struct {
    void *opaque_data;
    int user_tag;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Typedef for pointer types */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;
typedef int *int_ptr;

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    callback_func handler;
};
#endif

/* Additional complex types to ensure thorough traversal */

/* Nested struct with pointer to array */
struct GTY(()) container_struct {
    struct my_struct nested;
    int_array numbers;
    callback_func callback;
};

/* Union containing struct */
union GTY(()) complex_union {
    struct container_struct as_container;
    user_struct as_user;
};

#endif /* TEST_STRUCTURES_H */
