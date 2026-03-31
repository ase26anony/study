/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch cases in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2 GTY(());
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_string_1 field2;
  int *field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_struct_1 *next;
  my_struct_1 *prev;
  my_scalar_2 data;
};

struct GTY((skip)) my_struct_3 {
  void *opaque_data;
  int skip_count;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) my_user_struct_2 {
  double user_value;
  const char *user_name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  float as_float;
  void *as_ptr;
};

union GTY((desc("%0.as_int"))) my_union_2 {
  int as_int;
  double as_double;
  struct my_struct_1 *as_struct;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct_1 *my_pointer_1;
typedef my_struct_2 * GTY((skip)) my_pointer_2;
typedef const my_union_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5];
typedef const char *my_array_3[20] GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(my_struct_1 *, my_scalar_1);
typedef void (*my_callback_3)(void) GTY(());

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_id;
  struct lang_struct_type_1 *next;
  void *lang_data;
};

struct GTY((desc("%0.lang_id"), tag("LANG_STRUCT_2"))) lang_struct_type_2 {
  int lang_id;
  union my_union_1 lang_union;
  my_callback_1 callback;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) container_struct {
  my_array_1 fixed_array;
  my_pointer_1 dynamic_ptr;
  my_callback_1 handler;
  union my_union_2 data_union;
  struct GTY((user)) my_user_struct_1 user_data;
};

/* Nested anonymous struct/union */
struct GTY(()) nested_types {
  struct {
    int nested_field;
    char nested_char;
  } GTY(()) anonymous_struct;
  
  union {
    int as_int;
    struct my_struct_1 *as_ptr;
  } GTY(()) anonymous_union;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
