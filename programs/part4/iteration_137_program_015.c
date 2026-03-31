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
    struct undefined_type *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) my_user_struct {
    void *user_data;
    int user_id;
};

/* Another regular structure */
struct GTY(()) another_struct {
    my_struct *nested;
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
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func)(int, const char *);

/* Complex structure using multiple types */
struct GTY(()) complex_type {
    /* Scalar fields */
    my_scalar scalar_field;
    
    /* String field */
    my_string_type name;
    
    /* Pointer fields */
    my_struct_ptr struct_ptr;
    int_ptr int_ptr_field;
    
    /* Array field */
    int_array numbers;
    
    /* Callback field */
    callback_func handler;
    
    /* Union field */
    my_union data;
    
    /* Nested structure */
    another_struct nested;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* This mimics how GCC defines language-specific structures */
#ifdef GCC
struct GTY(()) lang_struct {
    int lang_specific_field;
    void *lang_data;
};
#endif

/* Simulate GCC's lang hooks structure definition */
#define DEFINE_LANG_STRUCT(name) \
    struct GTY(()) name { \
        int lang_id; \
        const char *lang_name; \
    }

DEFINE_LANG_STRUCT(lang_hooks_struct);

/* Additional pointer types for coverage */
typedef union my_union *union_ptr_t;
typedef callback_func *callback_ptr_t;

#endif /* TEST_STRUCTURES_H */
