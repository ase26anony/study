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
typedef void (*func_ptr_type)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);

/* A structure containing a callback */
struct GTY(()) struct_with_callback {
    callback_type handler;
    int state;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    void *lang_data;
};
#endif

/* Nested structure example */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        my_union inner_union;
    } nested;
    
    struct inner_struct *inner_ptr;
    int_array numbers;
};

/* Union containing structures */
union GTY(()) complex_union {
    struct GTY(()) {
        int type;
        void *data;
    } tagged;
    
    struct GTY(()) {
        int x, y;
    } point;
};

/* More scalar types for variety */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

typedef _Bool my_bool;
typedef long double my_long_double;

#endif /* TEST_STRUCTURES_H */
