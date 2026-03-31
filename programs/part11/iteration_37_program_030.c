/* test-gtype.h - Test file for gengtype type categorization coverage */
/* This file should be added to GTFILES in the GCC build system */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

/* Include necessary GCC headers */
#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef unsigned long my_unsigned_scalar GTY(());
typedef double my_float_scalar GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  void *field2;
  my_string_type field3;
};

/* Another struct for counting */
struct another_struct GTY(())
{
  struct my_base_struct *next;
  int data;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* User structs are typically those defined in plugins or extensions */
#define USER_STRUCT_MARKER 1
struct GTY((user)) my_user_struct
{
  int user_data;
  struct my_base_struct *link;
};

/* Alternative user struct definition */
struct another_user_struct GTY((user))
{
  long id;
  char *name GTY((string));
};

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int int_val;
  double double_val;
  void *ptr_val;
  struct my_base_struct *struct_ptr;
};

/* TYPE_POINTER: Pointer types with specific annotations */
typedef struct opaque_type *opaque_ptr_type GTY((ptr));
typedef void *generic_pointer GTY((ptr));

/* Forward declaration for pointer to incomplete type */
struct incomplete_struct;
typedef struct incomplete_struct *incomplete_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));

struct array_container GTY(())
{
  int count;
  int elements[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(const char *, int) GTY((callback));

struct callback_container GTY(())
{
  simple_callback cb1;
  complex_callback cb2;
};

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* These are typically tagged for specific language frontends */
struct GTY((tag("LANG"))) lang_specific_struct
{
  int lang_data;
  void *lang_private;
};

/* Another language struct with different tag */
struct GTY((tag("CPLUSPLUS"))) cxx_lang_struct
{
  struct lang_specific_struct *base;
  int cxx_specific;
};

/* TYPE_UNDEFINED: Incomplete/undefined types */
/* Forward declarations without definitions */
struct undefined_struct;
union undefined_union;

/* Types with malformed or incomplete GTY annotations */
struct GTY(()) partially_defined_struct;

/* This should trigger TYPE_UNDEFINED when referenced */
typedef struct undefined_struct *undefined_ptr GTY((ptr));

/* Complex nested type to ensure traversal */
struct complex_container GTY(())
{
  /* Contains multiple type categories */
  my_scalar_type scalar;
  my_string_type string;
  struct my_base_struct *nested_struct;
  union my_union_type nested_union;
  opaque_ptr_type opaque_ptr;
  simple_callback callback;
  int array_member[5];
  struct lang_specific_struct *lang_struct_ptr;
  struct undefined_struct *undefined_ptr;  /* This should count as TYPE_UNDEFINED */
};

/* Template-like structure for comprehensive coverage */
#define DECLARE_GTY_STRUCT(name) \
  struct name##_wrapper GTY(()) { \
    struct name *inner; \
    int wrapper_id; \
  }

/* Instantiate some template structures */
DECLARE_GTY_STRUCT(my_base_struct);
DECLARE_GTY_STRUCT(lang_specific_struct);

/* Enumeration type (should be treated as scalar) */
typedef enum my_enum GTY(())
{
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} my_enum_type;

/* Structure containing an enum */
struct enum_container GTY(())
{
  my_enum_type enum_field;
  int other_field;
};

/* Pointer chain for testing traversal */
struct pointer_chain GTY(())
{
  struct pointer_chain *next GTY((skip));
  void *data;
};

/* Self-referential structure */
struct self_ref_struct GTY(())
{
  int value;
  struct self_ref_struct *next;
  struct self_ref_struct *prev;
};

/* Union containing pointers to different types */
union type_selector GTY(())
{
  struct my_base_struct *as_struct;
  struct lang_specific_struct *as_lang_struct;
  struct undefined_struct *as_undefined;
  void *as_opaque;
};

#endif /* TEST_GTYPE_H */
