/* test_structures.h - Contains examples of all gengtype type categories */

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
struct GTY(()) regular_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *ptr;  /* Forward reference */
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_struct {
    void *data;
    int size;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef regular_struct *struct_ptr;
typedef int *int_ptr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef regular_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    callback_type handler;
};
#endif

/* Nested structure to ensure traversal */
struct GTY(()) outer_struct {
    regular_struct inner;
    my_union choice;
    int_array numbers;
    callback_type callback;
};

/* Another structure with pointer chain */
struct GTY(()) pointer_chain {
    struct_ptr next;
    int value;
};

#endif /* TEST_STRUCTURES_H */
