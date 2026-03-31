/* test_state_types.h - Header file with diverse GTY-marked types */
#ifndef TEST_STATE_TYPES_H
#define TEST_STATE_TYPES_H

/* TYPE_SCALAR: Simple scalar typedef */
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

/* TYPE_POINTER: Pointer typedef */
typedef struct my_struct *my_ptr GTY((skip));

/* TYPE_ARRAY: Array type */
typedef int my_array[10] GTY((skip));

/* TYPE_CALLBACK: Function pointer (callback) */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GCC
struct lang_struct GTY(())
{
  int lang_specific;
  void *lang_data GTY((skip));
};
#endif

/* Root structure to force processing of all types */
struct root_struct GTY((root))
{
  my_scalar scalar_field;
  my_string string_field;
  struct my_struct *struct_ptr;
  union my_union union_field;
  my_array array_field;
  my_callback callback_field;
  struct my_user_struct user_struct_field;
#ifdef GCC
  struct lang_struct *lang_struct_ptr;
#endif
  struct undefined_type *undefined_ptr;
};

#endif /* TEST_STATE_TYPES_H */
