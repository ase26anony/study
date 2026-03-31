/* test_structures.h - Contains examples of all gengtype type categories */

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

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: Struct with GTY((user)) */
struct GTY((user)) my_user_struct {
    void *data;
    int size;
};

/* Another regular struct */
struct GTY(()) another_struct {
    my_struct *ptr;
    int count;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* Nested struct to ensure traversal */
struct GTY(()) container {
    my_struct items[4];          /* TYPE_ARRAY inside struct */
    my_struct *next;             /* TYPE_POINTER inside struct */
    comparison_fn compare;       /* TYPE_CALLBACK inside struct */
    my_string_type name;         /* TYPE_STRING inside struct */
};

/* For language-specific structure */
#ifdef GCC
/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_field;
};
#endif

/* Global variable to force processing */
extern struct GTY(()) container *global_container;

#endif /* TEST_STRUCTURES_H */
