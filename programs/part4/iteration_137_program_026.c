/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    double field2;
    struct undefined_type *ptr;  /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) my_user_struct {
    int user_field;
    void *user_data;
};

/* Another regular struct for counting */
struct GTY(()) another_struct {
    my_scalar scalar_field;
    my_string_type string_field;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    callback_func handler;
};
#endif

/* Nested structure to ensure traversal */
struct GTY(()) container_struct {
    struct my_struct nested;
    union my_union data;
    int_array numbers;
    callback_func callback;
};

#endif /* TEST_STRUCTURES_H */
