/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef char *my_mutable_string;
typedef const char *const my_const_string_ptr;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_struct {
  int value;
  struct linked_struct *GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc ("%1.type"))) tagged_struct {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_value;
    my_string string_value;
  } GTY((desc ("%0.type"))) data;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *opaque_data;
  int user_id;
};

struct GTY((user)) another_user_struct {
  long custom_field;
  struct user_struct *related;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int int_val;
  float float_val;
  my_string string_val;
};

union GTY((desc ("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    int width;
    int height;
  } dimensions;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef my_union *union_pointer;
typedef user_struct *user_struct_pointer;
typedef const my_scalar *const_scalar_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];
typedef int multi_dim_array[2][3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*comparison_callback)(const void *, const void *);
typedef my_string (*string_generator)(void);
typedef void (*complex_callback)(my_struct *, my_union *, my_callback);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc ("%1.lang_type"), chain_next = "%h.next")) lang_struct_type {
  int lang_type;
  struct lang_struct_type *next;
  my_string name;
  union {
    int int_val;
    my_callback callback;
  } lang_data;
};

struct GTY((desc ("%1.kind"))) another_lang_struct {
  enum lang_kind { LANG_A, LANG_B, LANG_C } kind;
  union {
    struct lang_struct_type *lang_ptr;
    user_struct *user_ptr;
  } data;
  int lang_specific_field;
};

/* Additional types to ensure multiple instances */
struct GTY(()) extra_struct_one {
  my_array array_field;
  my_pointer ptr_field;
};

struct GTY(()) extra_struct_two {
  union_union_field;
  my_callback callback_field;
};

union GTY(()) extra_union {
  extra_struct_one struct_one;
  extra_struct_two struct_two;
  lang_struct_type lang_struct;
};

/* Pointer to callback */
typedef my_callback *callback_pointer;

/* Array of pointers */
typedef my_pointer *pointer_array[8];

/* Struct containing array of callbacks */
struct GTY(()) callback_container {
  my_callback callbacks[4];
  comparison_callback compare;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
