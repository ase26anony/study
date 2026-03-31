/* test_state_types.h - Header file with diverse GTY-marked types */
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

/* TYPE_CALLBACK: Function pointer (callback) */
typedef void (*my_callback)(int) GTY((skip));

/* TYPE_UNDEFINED: Forward declaration */
struct undefined_type;

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GCC
struct lang_struct GTY(())
{
  int lang_field1;
  void *lang_field2;
};
#endif

/* Additional pointer types for coverage */
typedef union my_union *union_ptr GTY((skip));
typedef my_callback *callback_ptr GTY((skip));

#endif /* TEST_STATE_TYPES_H */
