/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_plain_struct GTY(()) {
  int field1;
  my_scalar_type field2;
  void *field3 GTY((skip));
};

/* TYPE_USER_STRUCT: User-defined structure 
   This is typically a structure from plugin/user code */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_plain_struct *nested GTY((tag("0")));
};

/* TYPE_UNION: Union type */
union my_union_type GTY(()) {
  int as_int;
  void *as_ptr GTY((ptr));
  double as_double;
};

/* TYPE_POINTER: Pointer type with special handling */
typedef struct opaque_type *opaque_ptr_type GTY((ptr));

/* Forward declaration for opaque type */
struct opaque_type;

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array_type[] GTY((length("my_array_length")));
extern int my_array_length;

/* Another array type */
struct array_container GTY(()) {
  int count;
  int values[10] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_type)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific_struct {
  int lang_data;
  callback_func_type handler;
  union my_union_type variant;
};

/* TYPE_UNDEFINED: Incomplete/forward declared type without proper GTY */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* Another undefined case: struct with malformed GTY annotation */
struct GTY malformed_struct {
  int x;
};

/* Complex nested types to ensure traversal */
struct container_struct GTY(()) {
  /* Nested scalar */
  long nested_scalar;
  
  /* Nested string */
  const char *nested_string GTY((string));
  
  /* Nested pointer */
  struct lang_specific_struct *lang_ptr GTY((ptr));
  
  /* Nested array */
  float nested_array[5] GTY((length("5")));
  
  /* Nested callback */
  void (*nested_callback)(struct container_struct *) GTY((callback));
  
  /* Nested union */
  union {
    int tag;
    void *data GTY((ptr));
  } nested_union GTY((desc("tag")));
  
  /* Reference to user struct */
  struct my_user_struct *user_ref;
  
  /* Flexible array member */
  int flexible_member[] GTY((length("nested_scalar")));
};

/* Global variables using these types for additional coverage */
extern struct my_plain_struct GTY(()) *global_struct_ptr;
extern union my_union_type GTY(()) global_union;
extern callback_func_type GTY(()) global_callback;
extern struct lang_specific_struct GTY(()) *global_lang_struct;

/* Enum type (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY(());

/* Bitmask type */
typedef unsigned int my_bitmask GTY(());

/* Self-referential structure */
struct recursive_struct GTY(()) {
  int value;
  struct recursive_struct *next GTY((ptr));
  struct recursive_struct *prev GTY((ptr));
};

/* Variadic callback type */
typedef void (*variadic_callback)(int, ...) GTY((callback));

/* Array of pointers */
typedef struct my_plain_struct *struct_ptr_array[10] GTY(());

/* Multi-dimensional array */
typedef int matrix_type[3][4] GTY(());

/* Const pointer to const */
typedef const struct container_struct * const const_container_ptr GTY((ptr));

#endif /* TEST_GTYPE_H */
