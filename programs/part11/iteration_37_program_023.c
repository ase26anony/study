/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This is typically a structure from plugin/extension code */
struct GTY((user)) my_user_struct
{
  struct my_base_struct *base GTY((skip));
  int user_data;
  void (*user_func)(void) GTY((skip));
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(())
{
  int i;
  float f;
  void *p;
  my_string_type s;
};

/* TYPE_POINTER: Pointer type with ptr attribute */
typedef struct opaque_struct *opaque_ptr_type GTY((ptr));

/* Forward declaration for pointer type */
struct opaque_struct;

/* Another pointer type to incomplete type */
typedef union incomplete_union *incomplete_ptr GTY((skip));

/* TYPE_ARRAY: Array types */
typedef int fixed_array_type[10] GTY(());

/* Variable length array type */
struct array_container GTY(())
{
  int count;
  int elements GTY((length("%h.count"))) [];
};

/* Flexible array member */
typedef struct flexible_array_struct
{
  int len;
  char data GTY((length("%h.len"))) [];
} flexible_array_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_type)(int, const char *) GTY((callback));

/* Callback in a structure */
struct callback_container GTY(())
{
  callback_func_type handler;
  int state;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct GTY((tag("LANG"))) lang_specific_struct
{
  int lang_data;
  struct my_base_struct *lang_ptr;
  union my_union_type lang_union;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
/* These should be categorized as undefined */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* Malformed GTY annotation that might cause undefined categorization */
struct GTY((invalid_option)) potentially_undefined_struct
{
  int x;
};

/* Another undefined case: type with conflicting GTY attributes */
struct GTY((skip, desc("%0"))) conflicting_attrs_struct
{
  /* This combination might be treated as undefined */
  int data;
};

/* Complex nested types to ensure traversal */
struct complex_container GTY(())
{
  /* Contains multiple type categories */
  my_scalar_type scalar;
  my_string_type string;
  struct my_base_struct *struct_ptr;
  union my_union_type union_member;
  callback_func_type callback;
  int array_member[5];
  struct lang_specific_struct *lang_ptr;
  struct undefined_struct *undefined_ptr;  /* This should be TYPE_UNDEFINED */
};

/* Template-like macro usage (common in GCC internals) */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct name GTY(()) { \
    field_type data; \
    struct name *next; \
  }

/* Instantiate the macro for different types */
DEFINE_GTY_STRUCT(int_list, int);
DEFINE_GTY_STRUCT(string_list, my_string_type);

/* Enumeration type (should be treated as scalar for counting) */
typedef enum my_enum GTY(())
{
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} my_enum_type;

/* Bitmask type */
typedef unsigned int bitmask_type GTY(());

/* Self-referential structure */
struct recursive_struct GTY(())
{
  int value;
  struct recursive_struct *next GTY((skip));
  struct recursive_struct *prev GTY((skip));
};

/* Union with struct members */
union nested_union GTY(())
{
  struct {
    int x;
    int y;
  } point;
  struct {
    float r;
    float g;
    float b;
    float a;
  } color;
};

/* Array of pointers */
typedef struct my_base_struct *struct_ptr_array[10] GTY(());

/* Array of arrays */
typedef int matrix_type[3][4] GTY(());

/* Function pointer with complex signature */
typedef int (*complex_callback)(struct my_base_struct *,
                                union my_union_type,
                                callback_func_type) GTY((callback));

/* Language struct with callback */
struct GTY((tag("LANG"))) lang_with_callback
{
  complex_callback lang_callback;
  int lang_state;
};

/* Void pointer type */
typedef void *generic_ptr GTY((ptr));

/* Const pointer type */
typedef const struct my_base_struct *const_struct_ptr GTY((skip));

/* Pointer to array */
typedef int (*array_ptr_type)[5] GTY((skip));

/* Pointer to function returning pointer */
typedef struct my_base_struct *(*func_returning_ptr)(void) GTY((callback));

#endif /* TEST_GTYPE_H */
