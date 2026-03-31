/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   in gengtype.cc to ensure all switch cases are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String pointer typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)+1"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((desc("%0"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_struct_1 * GTY((tag("0"))) next;
  my_struct_1 * GTY((tag("1"))) prev;
  my_array_1 arr_field;
};

struct GTY((desc("%0"))) my_struct_3 {
  my_callback_1 callback;
  my_union_1 union_field;
  int extra_field;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  int user_field1;
  void *user_field2;
};

struct GTY((user)) my_user_struct_2 {
  my_scalar_1 data;
  const char *name;
};

struct GTY((user)) my_user_struct_3 {
  void *opaque_data;
  int flags;
};

/* TYPE_UNION: Union types */
union GTY((desc("%0"))) my_union_1 {
  my_scalar_1 as_scalar;
  my_string_1 as_string;
  my_struct_1 *as_struct;
};

union GTY((tag("0"))) my_union_2 {
  int int_val;
  double double_val;
  void *ptr_val;
};

union GTY((desc("%0"))) my_union_3 {
  my_array_1 as_array;
  my_callback_1 as_callback;
  struct {
    int nested_field1;
    int nested_field2;
  } nested;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct_1 *my_pointer_1;
typedef my_union_1 * GTY((skip)) my_pointer_2;
typedef const my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5];
typedef const char * GTY((length("strlen(%h)+1"))) my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char *);
typedef int (* GTY((skip)) my_callback_2)(my_struct_1 *);
typedef my_scalar_1 (*my_callback_3)(my_array_1, my_string_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_field1;
  my_string_1 lang_field2;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0"), tag("1"))) lang_struct_type_2 {
  my_callback_1 handler;
  my_union_2 data;
  int lang_specific;
};

struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_3 {
  int type_tag;
  void * GTY((skip)) lang_data;
  struct lang_struct_type_3 *next;
  struct lang_struct_type_3 *prev;
};

/* Additional struct to ensure TYPE_STRUCT counter increments multiple times */
struct GTY(()) extra_struct_1 {
  my_pointer_1 ptr;
  my_array_1 arr;
};

struct GTY((desc("%0"))) extra_struct_2 {
  my_user_struct_1 *user;
  my_callback_2 callback;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
