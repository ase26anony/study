/* test_state_types.h - Header file containing all type categories for gengtype state testing */

#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

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
  struct undefined_type *undef_ptr; /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: User-defined structure */
struct my_user_struct GTY((user))
{
  int user_data;
  void *user_ptr;
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
  int int_val;
  double double_val;
  void *ptr_val;
};

/* TYPE_POINTER: Pointer typedef */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_struct GTY(())
{
  int lang_specific;
  void *lang_data;
};
#endif

/* Another language-specific structure for broader coverage */
#if defined(GCC) || defined(__GNUC__)
struct another_lang_struct GTY(())
{
  int gcc_field;
  const char *gcc_name;
};
#endif

#endif /* TEST_STATE_TYPES_H */
