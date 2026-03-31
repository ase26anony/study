/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedef */
typedef int my_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ptr;
};

/* TYPE_USER_STRUCT: Struct with GTY((user)) */
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

/* TYPE_POINTER: Typedef for pointer */
typedef struct my_struct *my_struct_ptr;

/* TYPE_ARRAY: Array type definition */
typedef int my_array[10];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    void *lang_data;
};
#endif

/* Additional nested structures for comprehensive testing */
struct GTY(()) outer_struct {
    struct my_struct inner;
    union my_union choice;
    my_array numbers;
    callback_type callback;
};

/* Pointer to undefined type */
typedef struct undefined_type *undefined_ptr;

/* Another scalar type */
typedef unsigned long my_ulong;

#endif /* TEST_STRUCTURES_H */
