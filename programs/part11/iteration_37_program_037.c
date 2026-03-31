/* test-gtype.h - Test file for gengtype type categorization coverage */
/* This file should be added to GTFILES in the GCC build system */

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

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* Nested struct to ensure traversal */
struct outer_struct GTY(())
{
  struct my_base_struct inner GTY(());
  int extra_data;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* Defined with user marker to distinguish from regular structs */
struct GTY((user)) my_user_struct
{
  int user_id;
  void *user_data GTY((skip));
};

/* Another user struct with special handling */
struct GTY((user, desc("1"))) tagged_user_struct
{
  struct my_user_struct *next GTY((tag("0")));
  int counter;
};

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int as_int;
  double as_double;
  void *as_pointer GTY((ptr));
  struct my_base_struct *as_struct GTY((ptr));
};

/* Tagged union for special handling */
union GTY((tag("UNION_TAG"))) tagged_union
{
  int tag;
  struct
  {
    int x;
    int y;
  } point;
};

/* TYPE_POINTER: Various pointer types */
typedef struct opaque_struct *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((ptr));
typedef struct my_base_struct *struct_pointer GTY((ptr));

/* Forward declaration for pointer to incomplete type */
struct incomplete_type;
typedef struct incomplete_type *incomplete_pointer GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_base_struct struct_array[] GTY(());

/* Array of pointers */
typedef void *pointer_array[] GTY((ptr));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *, void *) GTY((callback));

/* Callback with parameters */
typedef void (*event_handler)(int event_id, void *user_data) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* Typically identified by tag or location in lang-specific dir */
struct GTY((tag("LANG"), desc("LANGUAGE_STRUCT"))) lang_specific_struct
{
  int lang_id;
  void *lang_data GTY((ptr));
  struct lang_specific_struct *next GTY((ptr));
};

/* Another language struct for C++ frontend simulation */
struct GTY((tag("CPLUSPLUS"), desc("CPP_CLASS"))) cpp_class_struct
{
  int vtable_offset;
  struct cpp_class_struct *parent GTY((ptr));
};

/* TYPE_UNDEFINED: Incomplete/undefined types */
/* Forward declarations without definition */
struct undefined_struct;
union undefined_union;

/* Types with malformed or incomplete GTY annotations */
typedef struct
{
  int x;
  int y;
} /* Missing GTY here */ unmarked_struct;

/* Function to force inclusion of undefined types */
void use_undefined_types(struct undefined_struct *s, union undefined_union *u);

/* Complex nested example to ensure deep traversal */
struct GTY(()) container_struct
{
  /* Scalar */
  int count GTY(());
  
  /* String */
  const char *name GTY((string));
  
  /* Struct */
  struct my_base_struct data GTY(());
  
  /* User struct */
  struct my_user_struct *user GTY((ptr));
  
  /* Union */
  union my_union_type value GTY(());
  
  /* Pointer */
  void *context GTY((ptr));
  
  /* Array */
  int scores[] GTY((length("count")));
  
  /* Callback */
  simple_callback handler GTY((callback));
  
  /* Language struct */
  struct lang_specific_struct *lang_data GTY((ptr));
  
  /* Pointer to undefined type */
  struct undefined_struct *future GTY((ptr));
};

/* Global variables of various types for root marking */
extern struct container_struct *global_container GTY((root));
extern my_string_type global_strings[] GTY((length("10")));
extern simple_callback global_callbacks[5] GTY(());

#endif /* TEST_GTYPE_H */
