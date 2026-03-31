/* test-gtype.h - Test file for covering gengtype type categorization */
/* This file should be added to GTFILES in the GCC build system */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type;
typedef unsigned int my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String pointer types with GTY((string)) */
typedef const char *my_string_type GTY((string));
typedef const char *const *string_array GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* Another struct for more coverage */
struct another_struct GTY(())
{
  struct my_base_struct *next GTY((skip));
  int data;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* Defined with user marker to differentiate from regular structs */
struct GTY((user)) my_user_struct
{
  int user_data;
  void *user_ptr GTY((ptr));
};

/* Alternative approach for user struct - in separate "user" context */
#define USER_GTY(x) GTY(x)

struct separate_user_struct USER_GTY(())
{
  long user_field;
};

/* TYPE_UNION: Union types marked with GTY */
union my_union_type GTY(())
{
  int int_value;
  double float_value;
  void *ptr_value GTY((ptr));
  my_string_type str_value;
};

/* TYPE_POINTER: Pointer types with various GTY options */
typedef struct my_base_struct *struct_ptr GTY((ptr));
typedef void *opaque_ptr GTY((ptr));
typedef int *int_ptr GTY((skip));  /* skip option still makes it a pointer type */

/* Forward declaration for pointer to incomplete type */
struct incomplete_struct;
typedef struct incomplete_struct *incomplete_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY((length("10")));
typedef int variable_array[] GTY((length("0")));  /* Flexible array member style */

/* Array of pointers */
typedef struct my_base_struct *ptr_array[5] GTY((length("5")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));

/* Callback with specific signature for language hooks */
typedef void (*lang_hook)(struct my_base_struct *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* Using tag option to mark as language-specific */
struct GTY((tag("LANG"))) lang_specific_struct
{
  int lang_data;
  simple_callback lang_callback;
  union my_union_type lang_union;
};

/* Another approach - struct in language-specific namespace */
struct cplusplus_struct GTY((tag("CPLUSPLUS")))
{
  struct lang_specific_struct *base GTY((ptr));
  int cxx_specific_field;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
/* These should be categorized as undefined */
struct forward_declared_struct;  /* No GTY markup - will be undefined */
extern struct forward_declared_struct *external_ref;  /* External reference */

/* Malformed GTY annotation that might cause undefined categorization */
struct GTY(()) malformed_struct
{
  /* Missing type specifier - will cause parsing issues */
  field_without_type;
};

/* Template-like structure that gengtype might not fully understand */
struct GTY(()) template_struct<T>  /* Invalid C but might parse oddly */
{
  T generic_field;
};

/* Self-referential pointer that might cause issues */
struct self_ref_struct GTY(())
{
  struct self_ref_struct *next GTY((ptr));
  /* Circular reference that might cause undefined behavior in gengtype */
  struct self_ref_struct *prev GTY((ptr));
};

/* Nested structures for complex type graph */
struct outer_container GTY(())
{
  struct inner_nested GTY(())
  {
    int inner_data;
    struct outer_container *parent GTY((ptr));
  } nested;
  
  struct inner_nested *nested_ptr GTY((ptr));
  variable_array flex_array;  /* Flexible array member */
};

/* Enumeration type (should be treated as scalar) */
typedef enum my_enum
{
  ENUM_VALUE1,
  ENUM_VALUE2,
  ENUM_VALUE3
} my_enum_type;

/* Bitfield structure */
struct bitfield_struct GTY(())
{
  unsigned int bitfield1 : 4;
  unsigned int bitfield2 : 8;
  unsigned int regular_field;
};

/* Union with struct members */
union complex_union GTY(())
{
  struct
  {
    int x;
    int y;
  } point;
  
  struct
  {
    float r;
    float g;
    float b;
    float a;
  } color;
};

/* Array of unions */
typedef union complex_union color_array[4] GTY((length("4")));

/* Callback that returns a pointer */
typedef struct my_base_struct *(*allocator_callback)(size_t) GTY((callback));

/* String array */
typedef const char *string_list[] GTY((string, length("0")));

/* Pointer to array */
typedef int (*array_ptr)[10] GTY((ptr));

/* Const pointer to const data */
typedef const int *const const_int_ptr GTY((ptr));

/* Volatile qualified pointer */
typedef volatile int *volatile_int_ptr GTY((ptr));

/* Restrict qualified pointer */
typedef int *restrict restrict_ptr GTY((ptr) GTY((restrict)));

/* Complete the incomplete struct declaration */
struct incomplete_struct GTY(())
{
  int now_complete;
  incomplete_ptr self_ptr;  /* Pointer to own type */
};

#endif /* TEST_GTYPE_H */
