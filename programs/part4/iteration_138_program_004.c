/* test_state_types.h - Header file containing all type categories for gengtype state coverage */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* TYPE_SCALAR: Simple typedef */
typedef unsigned int my_scalar GTY((skip));

/* TYPE_STRING: String type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular struct */
struct my_struct GTY(())
{
    my_scalar field1;
    my_string field2;
    int data;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct my_user_struct GTY((user))
{
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union definition */
union my_union GTY(())
{
    int int_val;
    float float_val;
    char *char_ptr;
};

/* TYPE_POINTER: Pointer typedef */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer typedef */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
    int lang_data;
    void *lang_ptr;
};
#endif

/* Additional pointer types for coverage */
typedef union my_union *union_ptr GTY((skip));
typedef my_array *array_ptr GTY((skip));

#endif /* TEST_STATE_TYPES_H */
