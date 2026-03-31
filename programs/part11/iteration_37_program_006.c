/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef double my_double_type GTY(());
typedef unsigned long my_ulong_type GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef const char *const *string_array GTY((string, length("0")));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_plain_struct GTY(()) {
  int field1;
  double field2;
  my_string_type field3;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* Defined in separate scope to be treated as user struct */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_plain_struct *nested GTY((skip));
};

/* Another approach for user struct - using special marker */
typedef struct GTY((desc("%1.user_tag"))) tagged_user_struct {
  int tag;
  void *data GTY((skip));
} user_struct_t;

/* TYPE_UNION: Union types */
union my_union_type GTY(()) {
  int as_int;
  double as_double;
  void *as_pointer GTY((ptr));
  struct my_plain_struct *as_struct;
};

/* TYPE_POINTER: Pointer types with various annotations */
typedef struct incomplete *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((ptr));
typedef const struct my_plain_struct *const_struct_ptr GTY((ptr));

/* Forward declaration for pointer to incomplete type */
struct forward_declared;
typedef struct forward_declared *forward_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_plain_struct *struct_ptr_array[] GTY((length("0")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *, void *) GTY((callback));
typedef void (*traverse_callback)(void *, void *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), desc("%0.lang_type"))) lang_specific_struct {
  int lang_data;
  union my_union_type lang_union;
  simple_callback lang_callback;
};

/* TYPE_UNDEFINED: Incomplete/malformed types */
/* Forward declaration without definition */
struct undefined_struct;
typedef struct undefined_struct undefined_type;

/* Malformed GTY annotation */
struct GTY((invalid_option)) malformed_struct {
  int x;
};

/* Another undefined case: type with conflicting annotations */
typedef struct GTY((ptr, skip)) conflicting_annotations *conflicting_ptr;

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
  /* Scalar */
  my_scalar_type scalar_field;
  
  /* String */
  my_string_type string_field;
  
  /* Struct */
  struct my_plain_struct struct_field;
  
  /* User struct */
  struct my_user_struct *user_struct_field GTY((skip));
  
  /* Union */
  union my_union_type union_field;
  
  /* Pointer */
  opaque_pointer pointer_field;
  
  /* Array */
  variable_array *array_field GTY((length("0")));
  
  /* Callback */
  filter_callback callback_field;
  
  /* Lang struct */
  struct lang_specific_struct *lang_struct_field GTY((ptr));
  
  /* Undefined */
  undefined_type *undefined_field GTY((skip));
};

/* Template-like macro to generate more cases */
#define DEFINE_GTY_TYPE(name, base) \
  typedef base name##_t GTY(());

DEFINE_GTY_TYPE(generated_int, int)
DEFINE_GTY_TYPE(generated_ptr, void *)

/* Enumeration type (should be treated as scalar) */
typedef enum GTY(()) {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type;

/* Bitmask type */
typedef unsigned int GTY(()) my_bitmask_type;

/* Self-referential structure */
struct GTY(()) self_ref_struct {
  int data;
  struct self_ref_struct *next GTY((ptr));
  struct self_ref_struct *prev GTY((ptr));
};

/* Circular reference between types */
struct GTY(()) type_a;
struct GTY(()) type_b;

struct type_a {
  int a_data;
  struct type_b *b_link GTY((ptr));
};

struct type_b {
  int b_data;
  struct type_a *a_link GTY((ptr));
};

/* Variable length structure */
struct GTY(()) var_len_struct {
  int length;
  int data[] GTY((length("%0.length")));
};

/* Opaque type with callbacks */
typedef struct GTY(()) {
  void *data GTY((skip));
  simple_callback cleanup;
  filter_callback filter;
} callback_container;

#endif /* TEST_GTYPE_H */
