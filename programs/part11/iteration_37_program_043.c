/* Test header for gengtype coverage testing
   This file defines various types to trigger all TYPE_* categories
   in the gengtype.cc switch statement (lines 182-213) */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY((scalar));

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_regular_struct GTY(()) {
  int field1;
  void *field2;
  my_scalar_type field3;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct GTY((incomplete));

/* TYPE_USER_STRUCT: User-defined structure 
   This is typically a struct defined in user code with special handling */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_regular_struct *nested;
};

/* Another approach for TYPE_USER_STRUCT using special marker */
#define USER_GTY(x) GTY((user)) x
struct USER_GTY(my_second_user_struct) {
  double user_value;
  my_string_type name;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(()) {
  int int_value;
  double double_value;
  void *pointer_value;
  my_string_type string_value;
};

/* TYPE_POINTER: Pointer type with ptr option */
typedef struct my_regular_struct *struct_ptr GTY((ptr));

/* TYPE_POINTER: Another pointer type to incomplete structure */
typedef struct undefined_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Fixed-size array */
typedef int fixed_array[10] GTY(());

/* TYPE_ARRAY: Variable-length array with length specifier */
struct array_container GTY(()) {
  int length;
  int elements GTY((length("%0.length")));
};

/* TYPE_ARRAY: Flexible array member */
struct flexible_struct GTY(()) {
  int count;
  int data GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*my_callback_type)(int, const char *) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*comparison_fn)(const void *, const void *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure 
   Using tag to identify as language-specific */
struct my_lang_struct GTY((tag("LANG_SPECIFIC"))) {
  int lang_data;
  void *lang_private;
};

/* Another language struct approach - often in langhooks.h style */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_chain {
  struct lang_chain *next;
  struct lang_chain *prev;
  int lang_id;
};

/* Complex nested structure to test traversal */
struct complex_container GTY(()) {
  /* TYPE_SCALAR */
  my_scalar_type scalar_field;
  
  /* TYPE_STRING */
  my_string_type string_field;
  
  /* TYPE_POINTER */
  struct_ptr struct_pointer;
  
  /* TYPE_POINTER to union */
  union my_union_type *union_ptr GTY((ptr));
  
  /* TYPE_ARRAY of scalars */
  int scalar_array[5];
  
  /* TYPE_ARRAY of pointers */
  struct my_regular_struct *struct_array[3] GTY((length("3")));
  
  /* TYPE_CALLBACK field */
  my_callback_type callback_field;
  
  /* Nested TYPE_STRUCT */
  struct nested_struct GTY(()) {
    int nested_data;
    opaque_ptr opaque_field;
  } nested;
  
  /* TYPE_UNION field */
  union my_union_type union_field;
};

/* Template for generating TYPE_UNDEFINED - malformed GTY annotation */
struct malformed_struct GTY((unknown_option)) {
  int bad_field;
};

/* Enum type (should be TYPE_SCALAR) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY((scalar));

/* Function pointer in struct (TYPE_CALLBACK in context) */
struct callback_container GTY(()) {
  char *name;
  comparison_fn compare;
};

/* Self-referential structure */
struct tree_node GTY((chain_next("%h.next"))) {
  int value;
  struct tree_node *next;
  struct tree_node *children GTY((length("0")));
};

/* Union with pointers */
union pointer_union GTY(()) {
  int *int_ptr;
  char **char_ptr_ptr;
  struct complex_container *container_ptr;
};

#endif /* TEST_GTYPE_H */
