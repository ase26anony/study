/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_plain_struct GTY(()) {
  int field1;
  my_scalar_type field2;
  struct my_plain_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: User-defined structure 
   This is typically a structure defined in user code (plugin) */
#define USER_STRUCT_MARKER
struct user_defined_struct GTY((user)) {
  int user_data;
  my_string_type user_name;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val GTY((skip));
  my_string_type str_val;
};

/* TYPE_POINTER: Pointer type to incomplete/generic type */
struct forward_declared;  /* Incomplete type */
typedef struct forward_declared *opaque_pointer GTY((ptr));

/* Another pointer type example */
typedef union my_union_type *union_ptr GTY((skip));

/* TYPE_ARRAY: Array types */
/* Fixed-size array */
typedef int fixed_array[10] GTY(());

/* Variable-length array (zero-length) */
struct array_container GTY(()) {
  int length;
  int flexible_array[] GTY((length("%0.length")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_function)(int, const char *) GTY((callback));

/* Callback in a structure */
struct callback_container GTY(()) {
  callback_function handler;
  void *user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct lang_specific_struct GTY((tag("LANG_SPECIFIC"))) {
  int lang_data;
  int lang_flags;
  struct lang_specific_struct *next_lang GTY((skip));
};

/* Another language struct with different tag */
struct cplusplus_struct GTY((tag("CPLUSPLUS"))) {
  void *vtable GTY((skip));
  int cpp_data;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
/* Forward declaration without GTY - will be TYPE_UNDEFINED */
struct undefined_struct;

/* Malformed GTY annotation */
struct malformed_gt_struct GTY(() {  /* Missing closing parenthesis */
  int bad_field;
};

/* Pointer to undefined type */
typedef struct undefined_struct *ptr_to_undefined GTY((skip));

/* Nested structures to ensure traversal */
struct outer_container GTY(()) {
  struct my_plain_struct nested_struct;
  union my_union_type nested_union;
  callback_function nested_callback;
  struct lang_specific_struct *lang_ptr GTY((skip));
};

/* Template-like structure for comprehensive coverage */
struct comprehensive_type GTY(()) {
  /* Scalar */
  int scalar_field;
  
  /* String */
  my_string_type string_field;
  
  /* Struct */
  struct my_plain_struct struct_field;
  
  /* Union */
  union my_union_type union_field;
  
  /* Pointer */
  opaque_pointer opaque_ptr;
  
  /* Array */
  fixed_array array_field;
  
  /* Callback */
  callback_function callback_field;
  
  /* Language struct */
  struct lang_specific_struct *lang_field GTY((skip));
  
  /* Self-reference */
  struct comprehensive_type *next GTY((skip));
};

/* Enumeration type (also scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY(());

/* Bitmask type */
typedef unsigned int bitmask_type GTY(());

/* Function pointer with arguments */
typedef int (*comparator_fn)(const void *, const void *) GTY((callback));

/* Structure with array of pointers */
struct pointer_array_container GTY(()) {
  int count;
  void *pointers[10] GTY((length("%0.count")));
};

/* Opaque structure declaration */
struct opaque_container;

/* Complete declaration elsewhere */
struct opaque_container GTY(()) {
  int hidden_data;
  struct undefined_struct *unknown_ptr GTY((skip));
};

#endif /* TEST_GTYPE_H */
