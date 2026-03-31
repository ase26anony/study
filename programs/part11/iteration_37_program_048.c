/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

/* Include necessary GCC headers */
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tree.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef unsigned long my_unsigned_scalar GTY(());
typedef double my_float_scalar GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const *string_array_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_plain_struct GTY(())
{
  int field1;
  void *field2 GTY((skip));
  tree tree_field GTY((tag("0")));
};

/* TYPE_USER_STRUCT: User-defined structure (from separate module/plugin) */
/* This will be recognized as user struct when processed from plugin context */
struct GTY((user)) my_user_struct
{
  int user_data;
  struct my_plain_struct *nested GTY((ptr));
};

/* Forward declaration to create TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY((ptr));

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int int_value;
  void *ptr_value;
  double float_value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_plain_struct *struct_ptr GTY((ptr));
typedef union my_union_type *union_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef int *scalar_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_plain_struct *struct_ptr_array[] GTY((ptr));

/* Flexible array member in a struct */
struct array_container GTY(())
{
  int count;
  int elements[] GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*typed_callback)(int, const char *) GTY((callback));
typedef tree (*tree_callback)(tree, tree) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific_struct
{
  int lang_data;
  tree lang_tree;
  struct lang_specific_struct *next GTY((ptr));
};

/* Nested structure to test deeper traversal */
struct outer_container GTY(())
{
  struct my_plain_struct inner GTY(());
  union my_union_type choice GTY(());
  struct outer_container *next GTY((ptr));
  simple_callback handler GTY((callback));
};

/* Template-like structure with conditional fields */
#ifdef ENABLE_FEATURE
struct conditional_struct GTY(())
{
  int enabled_field;
};
#else
struct conditional_struct GTY(())
{
  int disabled_field;
};
#endif

/* Another undefined type for counting */
typedef enum undefined_enum *undefined_enum_ptr GTY((ptr));

/* Self-referential structure */
struct recursive_struct GTY(())
{
  int value;
  struct recursive_struct *next GTY((ptr));
  struct recursive_struct *prev GTY((ptr));
};

/* Union containing pointers */
union pointer_union GTY(())
{
  struct my_plain_struct *struct_ptr GTY((ptr));
  struct lang_specific_struct *lang_ptr GTY((ptr));
  void *generic_ptr GTY((ptr));
};

/* Array of pointers */
typedef struct my_plain_struct *struct_ptr_array_10[10] GTY((ptr));

/* Callback with arguments */
typedef void (*complex_callback)(struct my_plain_struct *, 
                                 union my_union_type *,
                                 int) GTY((callback));

/* Mixed structure with all types */
struct comprehensive_struct GTY(())
{
  /* Scalar */
  int scalar_field;
  
  /* String */
  const char *string_field GTY((string));
  
  /* Pointer */
  struct my_plain_struct *struct_ptr GTY((ptr));
  
  /* Array */
  int array_field[5];
  
  /* Union */
  union my_union_type union_field GTY(());
  
  /* Callback */
  simple_callback cb_field GTY((callback));
  
  /* Nested structure */
  struct {
    int nested_scalar;
    void *nested_ptr GTY((ptr));
  } nested GTY(());
  
  /* Flexible array */
  int flexible_array[] GTY((length("0")));
};

#endif /* TEST_GTYPE_H */
