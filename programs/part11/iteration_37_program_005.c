/* test-gtype.h - Comprehensive test file for gengtype type categorization */
/* This file should be placed in gcc/test-gtype/ directory */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const constant_string GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* Nested structure to test traversal */
struct nested_struct GTY(())
{
  struct my_base_struct *base GTY((skip));
  int depth;
};

/* TYPE_USER_STRUCT: User-defined structure (defined in separate module/plugin) */
/* This requires special handling - often structures from plugins are user structs */
struct user_defined_struct GTY((user))
{
  int user_data;
  void *user_ptr GTY((ptr));
};

/* Forward declaration to create TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int int_val;
  double double_val;
  my_string_type string_val;
  void *ptr_val GTY((ptr));
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_base_struct *struct_ptr GTY((ptr));
typedef union my_union_type *union_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef int *int_ptr GTY((ptr));

/* Opaque pointer to incomplete type */
struct forward_declared;
typedef struct forward_declared *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_base_struct *struct_array[] GTY((ptr));

/* Flexible array member in a struct */
struct with_flex_array GTY(())
{
  int count;
  int data[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *input, char *output) GTY((callback));
typedef void (*struct_callback)(struct my_base_struct *s) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Typically identified by tag or location in lang-specific directory */
struct lang_specific_struct GTY((tag("LANG_SPECIFIC")))
{
  int lang_data;
  void *lang_private GTY((skip));
};

/* Another approach for lang struct - use special marker */
struct cplusplus_struct GTY((desc("%1")))
{
  int cpp_magic;
};

/* Complex nested type to ensure full traversal */
struct complex_container GTY(())
{
  /* Scalar */
  int id;
  
  /* String */
  my_string_type name;
  
  /* Struct pointer */
  struct my_base_struct *base_ptr GTY((ptr));
  
  /* Union */
  union my_union_type value;
  
  /* Array */
  int scores[5];
  
  /* Callback */
  simple_callback handler;
  
  /* Nested lang struct */
  struct lang_specific_struct *lang_data GTY((ptr));
};

/* Template-like structure (common in GCC internals) */
struct template_struct GTY(())
{
  int code;
  union
  {
    my_scalar_type scalar_val;
    my_string_type string_val;
    struct my_base_struct *struct_val GTY((ptr));
  } GTY((desc ("%0.code"))) u;
};

/* Incomplete type that should remain TYPE_UNDEFINED */
struct undefined_struct
{
  /* No definition provided - this should stay undefined */
  int *opaque_data;
};

/* Chain of pointers for testing */
typedef struct pointer_chain GTY(())
{
  struct pointer_chain *next GTY((ptr));
  void *data GTY((ptr));
} pointer_chain;

/* Enumeration type (should be treated as scalar) */
typedef enum my_enum GTY(())
{
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} my_enum_type;

/* Bitmask type */
typedef unsigned int bitmask GTY(());

/* Self-referential structure */
struct tree_node GTY(())
{
  int type;
  struct tree_node *left GTY((ptr));
  struct tree_node *right GTY((ptr));
  my_string_type name;
};

/* Variadic callback */
typedef void (*error_callback)(const char *format, ...) GTY((callback));

/* Array of callbacks */
typedef simple_callback callback_array[] GTY((length("0")));

/* Structure with array of strings */
struct string_array_container GTY(())
{
  int count;
  my_string_type strings[] GTY((length("count")));
};

#endif /* TEST_GTYPE_H */
