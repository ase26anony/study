/* test_state_types.h - Header file containing all type categories for gengtype state testing */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_SCALAR: A simple scalar type */
typedef unsigned int my_scalar GTY((skip));

/* TYPE_STRING: A string type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: A regular structure */
struct my_struct GTY(()) {
    my_scalar field1;
    my_string field2;
};

/* TYPE_USER_STRUCT: A user-defined structure */
struct my_user_struct GTY((user)) {
    int user_field;
};

/* TYPE_UNION: A union type */
union my_union GTY(()) {
    int int_val;
    my_scalar scalar_val;
    void *ptr_val;
};

/* TYPE_POINTER: A pointer type */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: An array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: A function pointer (callback) type */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(()) {
    int lang_field;
};
#endif

/* TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* Additional pointer types for coverage */
typedef union my_union *union_ptr GTY((skip));
typedef my_callback *callback_ptr GTY((skip));

#endif /* TEST_STATE_TYPES_H */
