/* test_state_types.h - Header file containing all type categories for gengtype state coverage */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_SCALAR: Simple scalar type */
typedef unsigned int my_scalar GTY((skip));

/* TYPE_STRING: String type with string marker */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structure */
struct my_struct GTY(())
{
    my_scalar field1;
    my_string field2;
    int field3;
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
    char char_val;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_UNDEFINED: Forward declaration (undefined type) */
struct undefined_type;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
    int lang_field1;
    void *lang_field2;
};
#endif

/* Additional complex types to ensure thorough coverage */
struct nested_struct GTY(())
{
    struct my_struct *nested_ptr;
    union my_union nested_union;
    my_array nested_array;
};

#endif /* TEST_STATE_TYPES_H */
