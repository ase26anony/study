/* test_state_types.h - Header file containing all type categories for gengtype state testing */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* 1. TYPE_SCALAR: Simple scalar type */
typedef unsigned int my_scalar GTY((skip));

/* 2. TYPE_STRING: String type */
typedef const char *my_string GTY((string));

/* 3. TYPE_STRUCT: Regular structure */
struct my_struct GTY(())
{
    my_scalar field1;
    my_string field2;
};

/* 4. TYPE_USER_STRUCT: User-defined structure */
struct my_user_struct GTY((user))
{
    int user_field;
};

/* 5. TYPE_UNION: Union type */
union my_union GTY(())
{
    int int_val;
    my_scalar scalar_val;
    void *ptr_val;
};

/* 6. TYPE_POINTER: Pointer type */
typedef struct my_struct *my_ptr GTY((skip));

/* 7. TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* 8. TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*my_callback)(int) GTY((skip));

/* 9. TYPE_UNDEFINED: Forward declaration (undefined type) */
struct undefined_type;

/* 10. TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
    int lang_field;
};
#endif

/* For testing without GCC defined, create an alternative */
#ifndef GCC
struct lang_struct GTY(())
{
    int lang_field;
};
#endif

#endif /* TEST_STATE_TYPES_H */
