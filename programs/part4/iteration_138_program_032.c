/* test_state_types.h - Header file containing all type categories for gengtype state coverage */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* TYPE_SCALAR: Simple typedef */
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

/* TYPE_CALLBACK: Function pointer (callback) */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
  int lang_specific;
  void *lang_data GTY((skip));
};
#endif

/* Another language-specific structure for coverage */
#if defined(GCC) || defined(LANG_SPECIFIC)
struct another_lang_struct GTY(())
{
  double lang_value;
  my_callback callback;
};
#endif

/* Complex nested type for thorough testing */
struct complex_nested GTY(())
{
  struct my_struct *nested_ptr GTY((skip));
  union my_union nested_union;
  my_array array_field;
  my_callback handler;
};

#endif /* TEST_STATE_TYPES_H */
