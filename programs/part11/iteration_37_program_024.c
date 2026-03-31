/* test-gtype.h - Test file for gengtype type categorization coverage */
/* This file should be placed in gcc/test-gtype/test-gtype.h */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_regular_struct GTY(()) {
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* TYPE_USER_STRUCT: User-defined structure 
   This is typically a structure defined in user code or plugin */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_regular_struct *link GTY((skip));
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  my_string_type str_val;
};

/* TYPE_POINTER: Pointer to incomplete/opaque type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_type GTY((ptr));

/* Another pointer type example */
typedef union my_union_type *union_ptr_type GTY((skip));

/* TYPE_ARRAY: Array types */
/* Fixed-size array */
typedef int fixed_array_type[10] GTY(());

/* Variable-length array (zero-length) */
struct array_container GTY(()) {
  int length;
  int elements GTY((length("%0.length")));
};

/* Flexible array member */
struct flex_array_struct GTY(()) {
  int count;
  int data[] GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func_type)(int, const char *) GTY((callback));

/* Callback in a structure */
struct callback_container GTY(()) {
  callback_func_type handler;
  int state;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct GTY((tag("LANG"))) lang_specific_struct {
  int lang_data;
  void *lang_private;
};

/* Another language struct with chain next */
struct GTY((tag("LANG"), chain_next("%h.next"))) lang_chain_struct {
  int value;
  struct lang_chain_struct *next;
};

/* TYPE_UNDEFINED: Forward declaration without complete definition 
   or malformed GTY annotation */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* Malformed GTY annotation to trigger undefined type */
struct GTY((invalid_option)) malformed_struct {
  int x;
};

/* Nested structures to test traversal */
struct outer_container GTY(()) {
  struct my_regular_struct regular;
  struct GTY((user)) my_user_struct *user_ptr;
  union my_union_type union_member;
  struct lang_specific_struct *lang_ptr;
  callback_func_type callback;
  struct array_container array_wrapper;
};

/* Template-like structure with conditional fields */
struct GTY((desc("%1.type"))) variant_struct {
  enum { INT_TYPE, STRING_TYPE, PTR_TYPE } type;
  union {
    int int_value;
    my_string_type string_value;
    void *ptr_value;
  } data;
};

/* Test structure with nested arrays */
struct nested_arrays GTY(()) {
  int matrix[3][4];
  struct flex_array_struct *flex;
};

/* Structure with self-reference */
struct self_ref_struct GTY(()) {
  int id;
  struct self_ref_struct *next GTY((skip));
  struct self_ref_struct *prev GTY((skip));
};

/* Union with pointers */
union pointer_union GTY(()) {
  int *int_ptr;
  struct my_regular_struct *struct_ptr;
  struct lang_specific_struct *lang_ptr;
};

/* Array of pointers */
typedef struct my_regular_struct *struct_ptr_array[5] GTY(());

/* Callback with context */
typedef void (*context_callback)(void *context, int value) GTY((callback));

struct callback_with_context GTY(()) {
  context_callback cb;
  void *context;
};

/* Language structure with inheritance-like pattern */
struct GTY((tag("LANG_BASE"))) lang_base {
  int base_data;
};

struct GTY((tag("LANG_DERIVED"))) lang_derived {
  struct lang_base base;
  int derived_data;
};

/* Test for param_is option */
struct GTY((param_is("@param"))) param_struct {
  int param;
};

/* Structure with alt_macro option */
struct GTY((alt_macro)) alt_struct {
  int alternative;
};

#endif /* TEST_GTYPE_H */
