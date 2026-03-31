/* test_state_types.h - Header file containing all type categories for gengtype state coverage */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already included elsewhere */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_SCALAR: Simple scalar type */
typedef unsigned int my_scalar GTY((skip));

/* TYPE_STRING: String type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structure */
struct my_struct GTY(())
{
    my_scalar field1;
    my_string field2;
};

/* TYPE_USER_STRUCT: User-defined structure */
struct my_user_struct GTY((user))
{
    int user_field1;
    char user_field2;
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
    int int_val;
    float float_val;
    char *char_ptr;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_UNDEFINED: Forward declaration (undefined type) */
struct undefined_type;

/* Language-specific structure - TYPE_LANG_STRUCT */
/* This will be conditionally compiled to ensure it's treated as language-specific */
#ifdef GCC
struct lang_struct GTY(())
{
    int lang_field1;
    void *lang_field2;
};
#endif

/* Root structure that references many of these types */
struct root_struct GTY((root))
{
    my_scalar scalar_field;
    my_string string_field;
    struct my_struct *struct_ptr;
    union my_union union_field;
    my_array array_field;
    my_callback callback_field;
    struct undefined_type *undefined_ptr;  /* Pointer to undefined type */
#ifdef GCC
    struct lang_struct *lang_struct_ptr;
#endif
};

#endif /* TEST_STATE_TYPES_H */
