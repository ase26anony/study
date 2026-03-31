/* test_state_types.h - Header file containing all type categories for gengtype state testing */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* 1. TYPE_SCALAR: Simple typedef */
typedef unsigned int my_scalar GTY((skip));

/* 2. TYPE_STRING: String type */
typedef const char *my_string GTY((string));

/* 3. TYPE_STRUCT: Regular struct */
struct my_struct GTY(())
{
    my_scalar field1;
    my_string field2;
};

/* 4. TYPE_USER_STRUCT: User-defined struct */
struct my_user_struct GTY((user))
{
    int user_field1;
    char user_field2;
};

/* 5. TYPE_UNION: Union definition */
union my_union GTY(())
{
    int int_val;
    float float_val;
    char *char_ptr;
};

/* 6. TYPE_POINTER: Pointer typedef */
typedef struct my_struct *my_ptr GTY((skip));

/* 7. TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* 8. TYPE_CALLBACK: Function pointer typedef */
typedef void (*my_callback)(int) GTY((skip));

/* 9. TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
    int lang_field;
    void *lang_data;
};
#endif

/* 10. TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* 11. Additional pointer types for coverage */
typedef union my_union *union_ptr GTY((skip));
typedef my_array *array_ptr GTY((skip));

#endif /* TEST_STATE_TYPES_H */
