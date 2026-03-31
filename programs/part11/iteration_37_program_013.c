/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef double my_double_type GTY(());
typedef unsigned long my_ulong_type GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string_type GTY((string));
typedef const char *const constant_string_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures */
struct my_base_struct GTY(()) {
  int field1;
  double field2;
  my_scalar_type field3;
};

/* Another struct for more coverage */
struct another_struct GTY(()) {
  struct my_base_struct *next GTY((skip));
  int data;
};

/* TYPE_USER_STRUCT: User-defined structure */
/* This is typically a structure from plugin/extension code */
struct GTY((user)) my_user_struct {
  int user_data;
  void *user_pointer GTY((ptr));
};

/* Forward declaration to create TYPE_UNDEFINED */
struct incomplete_struct;
typedef struct incomplete_struct *incomplete_ptr GTY((ptr));

/* TYPE_UNION: Union types */
union my_union_type GTY(()) {
  int as_int;
  double as_double;
  void *as_pointer GTY((ptr));
  const char *as_string GTY((string));
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_base_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef int *int_ptr GTY((ptr));
typedef union my_union_type *union_ptr GTY((ptr));

/* Opaque pointer type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_type GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_base_struct struct_array[] GTY(());

/* Array of pointers */
typedef void *pointer_array[] GTY((ptr));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*int_callback)(int, double) GTY((callback));
typedef void (*struct_callback)(struct my_base_struct *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct GTY((tag("LANG"))) lang_specific_struct {
  int lang_data;
  void *lang_pointer GTY((ptr));
  const char *lang_name GTY((string));
};

/* Another language struct with different tag */
struct GTY((tag("CPLUSPLUS"))) cxx_struct {
  int cxx_data;
  struct lang_specific_struct *lang_ref GTY((ptr));
};

/* TYPE_UNDEFINED: Create undefined types */
/* Forward declared struct without definition */
struct undefined_struct;
typedef struct undefined_struct undefined_type;

/* Malformed GTY annotation */
struct GTY((invalid_option)) bad_struct {
  int x;
};

/* Nested structures for complex testing */
struct container_struct GTY(()) {
  /* Scalar */
  int count GTY(());
  
  /* String */
  const char *name GTY((string));
  
  /* Struct pointer */
  struct my_base_struct *base GTY((ptr));
  
  /* Union */
  union my_union_type data GTY(());
  
  /* Array */
  int values[] GTY((length("count")));
  
  /* Callback */
  simple_callback callback GTY((callback));
  
  /* Language struct */
  struct lang_specific_struct *lang_struct GTY((ptr));
};

/* Template-like macro for generating multiple types */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct GTY(()) name { \
    field_type data; \
    struct name *next GTY((ptr)); \
  }

DEFINE_GTY_STRUCT(generated_struct_int, int);
DEFINE_GTY_STRUCT(generated_struct_ptr, void *);

/* Enum type (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY(());

/* Complex nested type */
struct complex_nested GTY(()) {
  struct {
    int x;
    int y;
  } point GTY(());
  
  union {
    int i;
    float f;
  } value GTY(());
  
  struct complex_nested *children[5] GTY((ptr));
};

/* Variable length struct with array at end */
struct var_len_struct GTY(()) {
  int length;
  char data[] GTY((length("length")));
};

#endif /* TEST_GTYPE_H */
