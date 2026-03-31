/* test_structures.h - Contains examples of all gengtype type categories */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedef */
typedef int my_scalar;
typedef unsigned long another_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *undef_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with GTY((user)) */
struct GTY((user)) user_struct {
    void *data;
    int size;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int int_val;
    float float_val;
    my_string_type str_val;
};

/* TYPE_POINTER: Typedef for pointer */
typedef struct my_struct *struct_ptr;
typedef union my_union *union_ptr;

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Another struct to ensure multiple instances are counted */
struct GTY(()) another_struct {
    callback_type callback;
    int_array numbers;
};

/* Nested structure for complex testing */
struct GTY(()) container {
    struct my_struct *first;
    union my_union second;
    struct_array items;
    callback_type handler;
};

#endif /* TEST_STRUCTURES_H */
