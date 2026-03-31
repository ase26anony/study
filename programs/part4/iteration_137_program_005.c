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
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct my_struct GTY(()) {
    int field1;
    double field2;
    struct my_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct user_defined GTY((user)) {
    void *data;
    int tag;
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *struct_ptr;
typedef int *int_ptr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific GTY(()) {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structure for additional coverage */
struct outer_struct GTY(()) {
    struct inner_struct GTY(()) {
        int nested_field;
        struct inner_struct *next;
    } inner;
    
    union nested_union GTY(()) {
        int a;
        float b;
    } uni;
};

/* Another structure with pointer fields */
struct with_pointers GTY(()) {
    struct my_struct *ptr1;
    struct undefined_type *ptr2;  /* Pointer to undefined type */
    my_string str_field;          /* String field */
    int_array array_field;        /* Array field */
    comparison_fn callback_field; /* Callback field */
};

#endif /* TEST_STRUCTURES_H */
