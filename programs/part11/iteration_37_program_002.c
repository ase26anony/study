/* Test header for gengtype coverage - defines all TYPE_* categories */

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
extern const char *global_string GTY((string));

/* TYPE_STRUCT: Plain C structures */
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
  int data[10];
};

/* TYPE_USER_STRUCT: User-defined structure 
   This is typically a struct from client code/plugin */
#define USER_STRUCT_MARKER 1
struct user_defined_struct GTY((user))
{
  int user_id;
  const char *user_name GTY((string));
  void *user_data GTY((skip));
};

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int as_int;
  double as_double;
  void *as_pointer GTY((ptr));
  const char *as_string GTY((string));
};

/* TYPE_POINTER: Pointer types with various attributes */
typedef struct opaque_struct *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((ptr));
typedef struct my_base_struct *struct_pointer GTY((skip));

/* Forward declaration for pointer to incomplete type */
struct incomplete_type;
typedef struct incomplete_type *incomplete_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[100] GTY(());
typedef int variable_array[] GTY((length("0")));
extern int external_array[] GTY(());

struct with_array GTY(())
{
  int count;
  variable_array items;  /* Flexible array member */
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *input GTY((string)), 
                               void *context GTY((skip))) GTY((callback));
typedef void (*user_callback)(struct user_defined_struct *user GTY((skip))) 
  GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific_struct GTY((tag("LANG_STRUCT")))
{
  int lang_id;
  void *lang_data GTY((ptr));
  struct lang_specific_struct *next GTY((skip));
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types 
   that will be categorized as undefined */
struct undefined_struct;
union undefined_union;
enum undefined_enum;

/* Malformed GTY annotation that might cause undefined type */
struct problematic_struct GTY((invalid_option))
{
  int x;
};

/* Complex nested type to ensure traversal */
struct container_struct GTY(())
{
  /* Scalar */
  my_scalar_type scalar_field;
  
  /* String */
  my_string_type string_field;
  
  /* Struct */
  struct my_base_struct nested_struct;
  
  /* User struct */
  struct user_defined_struct user_struct_field;
  
  /* Union */
  union my_union_type union_field;
  
  /* Pointer */
  opaque_pointer pointer_field;
  
  /* Array */
  fixed_array array_field;
  
  /* Callback */
  simple_callback callback_field;
  
  /* Lang struct */
  struct lang_specific_struct *lang_struct_ptr GTY((skip));
  
  /* Self-reference */
  struct container_struct *next GTY((skip));
  
  /* Pointer to undefined type */
  struct undefined_struct *undefined_ptr GTY((ptr));
};

/* Global variables for root marking */
extern struct container_struct *global_container GTY((root));
extern union my_union_type global_union GTY(());
extern simple_callback global_callback GTY((callback));

#endif /* TEST_GTYPE_H */
