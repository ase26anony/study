/* test_state_types.h - Header file containing all type categories for gengtype state testing */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

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
  int user_field;
  void *user_data GTY((skip));
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
  int int_val;
  float float_val;
  void *ptr_val GTY((skip));
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_UNDEFINED: Forward declaration (undefined type) */
struct undefined_type;

/* Language-specific structure (TYPE_LANG_STRUCT) */
/* This will be conditionally compiled based on GCC definition */
#ifdef GCC
struct lang_struct GTY(())
{
  int lang_field;
  struct my_struct *next GTY((skip));
};
#endif

/* Additional structure to ensure TYPE_LANG_STRUCT is processed */
/* Using a macro that gengtype recognizes as language-specific */
#define GTY_LANG_STRUCT struct lang_struct2 GTY(())
GTY_LANG_STRUCT
{
  int lang_field2;
};

#endif /* TEST_STATE_TYPES_H */
