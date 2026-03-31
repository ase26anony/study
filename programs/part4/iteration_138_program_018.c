/* test_state_types.h - Header file containing all type categories for gengtype state coverage */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* TYPE_SCALAR: Simple scalar type */
typedef unsigned int my_scalar GTY((skip));

/* TYPE_STRING: String type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structure */
struct my_struct GTY(())
{
  my_scalar value;
  my_string name;
};

/* TYPE_USER_STRUCT: User-defined structure */
struct my_user_struct GTY((user))
{
  int data;
  void *extra GTY((skip));
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
  int int_val;
  float float_val;
  char *char_ptr GTY((skip));
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
  int lang_specific_field;
};
#endif

/* Additional complex types to ensure thorough coverage */
struct nested_struct GTY(())
{
  struct my_struct inner;
  union my_union choice;
  my_array numbers;
};

/* Variant union for tagged unions */
union variant_union GTY((desc ("$1.type")))
{
  struct
  {
    int type;
  } common;
  struct
  {
    int type;
    int int_value;
  } int_case;
  struct
  {
    int type;
    double double_value;
  } double_case;
};

#endif /* TEST_STATE_TYPES_H */
