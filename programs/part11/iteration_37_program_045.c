/* Test header for gengtype coverage testing */
/* This file defines various types to exercise all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

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

/* Another struct with pointer members */
struct struct_with_pointers GTY(())
{
  struct my_base_struct *next GTY((skip));
  void *data GTY((ptr));
  int count;
};

/* TYPE_USER_STRUCT: Structures with user-defined markers */
/* Often user structs are defined with special GTY options */
struct user_defined_struct GTY((user))
{
  int user_id;
  const char *user_name GTY((string));
};

/* Alternative approach: struct defined in "user" context */
#define USER_GTY(x) GTY(x)
struct another_user_struct USER_GTY(())
{
  long user_data;
  struct user_defined_struct *link GTY((skip));
};

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int as_int;
  double as_double;
  void *as_pointer GTY((ptr));
  struct my_base_struct *as_struct GTY((skip));
};

/* Tagged union variant */
union tagged_union GTY((desc("tag_field")))
{
  int tag_field;
  struct {
    int type;
    union my_union_type data;
  } variant;
};

/* TYPE_POINTER: Pointer types with special handling */
typedef struct opaque_struct *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((ptr));
typedef const void *const_pointer GTY((ptr));

/* Forward declaration for pointer usage */
struct forward_declared;
typedef struct forward_declared *forward_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_base_struct struct_array[] GTY((length("count")));

/* Array within a struct */
struct array_container GTY(())
{
  int count;
  int elements[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *input GTY((string)), 
                               void *context GTY((ptr))) GTY((callback));

/* Callback with parameters */
typedef struct my_base_struct* (*allocator_callback)(size_t size) 
  GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* Typically identified by special tags or being in lang-specific dirs */
struct c_lang_struct GTY((tag("LANG_C")))
{
  int c_specific;
  void *tree_node GTY((ptr));
};

struct cplusplus_lang_struct GTY((tag("LANG_CPLUSPLUS")))
{
  int cpp_specific;
  struct c_lang_struct *base GTY((skip));
};

/* TYPE_UNDEFINED: Incomplete/forward declared types */
/* These should be categorized as undefined */
struct incomplete_struct;
union incomplete_union;

/* Malformed GTY annotation (might cause undefined type) */
struct potentially_undefined GTY(());  /* Forward declaration */

/* Another undefined case: type with invalid GTY options */
typedef int (*bad_callback)() /* Missing GTY((callback)) */;

/* Complex nested example combining multiple types */
struct complex_nested GTY(())
{
  /* Scalar */
  int id;
  
  /* String */
  const char *name GTY((string));
  
  /* Struct pointer */
  struct my_base_struct *base GTY((skip));
  
  /* Union */
  union my_union_type value;
  
  /* Array */
  int scores[] GTY((length("score_count")));
  int score_count;
  
  /* Callback */
  simple_callback notify GTY((callback));
  
  /* Pointer to lang struct */
  struct c_lang_struct *lang_info GTY((ptr));
  
  /* User struct */
  struct user_defined_struct user_data;
};

/* Template for generating many instances */
#define DECLARE_STRUCT(num) \
  struct generated_struct_##num GTY(()) { \
    int id_##num; \
    struct generated_struct_##num *next GTY((skip)); \
  }

/* Generate several structs to ensure thorough processing */
DECLARE_STRUCT(1);
DECLARE_STRUCT(2);
DECLARE_STRUCT(3);
DECLARE_STRUCT(4);
DECLARE_STRUCT(5);

/* Enumeration type (should be scalar) */
typedef enum {
  STATE_INIT,
  STATE_PROCESSING,
  STATE_DONE
} process_state GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(())
{
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Self-referential structure */
struct linked_node GTY(())
{
  int data;
  struct linked_node *next GTY((skip));
  struct linked_node *prev GTY((skip));
};

/* Circular reference between types */
struct type_a GTY(())
{
  int a_data;
  struct type_b *b_link GTY((skip));
};

struct type_b GTY(())
{
  int b_data;
  struct type_a *a_link GTY((skip));
};

#endif /* TEST_GTYPE_H */
