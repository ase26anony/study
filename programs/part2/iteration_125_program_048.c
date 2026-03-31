/* Test types for gengtype coverage testing.
   This file defines types corresponding to each enum type_kind case
   in gengtype.cc to ensure all switch cases are executed. */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef const char *another_string;
typedef const char *test_string;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY((skip)) another_struct {
  double d_field;
  int i_field;
  const char *name;
};

struct GTY((chain_next = "%h.next")) linked_struct {
  int value;
  struct linked_struct *GTY((skip)) next;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_type {
  void *data;
  int tag;
};

struct GTY((user)) another_user_struct {
  long id;
  void *user_data;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int as_int;
  double as_double;
  void *as_pointer;
};

union GTY((desc("%0.tag"))) tagged_union {
  int tag;
  struct {
    int type;
    void *data;
  } GTY((skip)) complex;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef another_struct *another_pointer;
typedef user_struct_type *user_struct_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(my_struct *, my_string);
typedef void (*complex_callback)(my_callback, int);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
  void *lang_data;
};

struct GTY((desc("%0.type"), tag("LANG_TYPE"))) another_lang_struct {
  enum { LANG_A, LANG_B } type;
  union {
    int int_val;
    double double_val;
  } GTY((skip)) value;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with pointer array */
struct GTY(()) complex_nested {
  my_array numbers;
  my_pointer ptr;
  my_callback cb;
  union {
    int i;
    double d;
  } GTY((skip)) choice;
};

/* Struct with all type kinds */
struct GTY(()) all_types_struct {
  /* SCALAR */
  my_scalar scalar_field;
  
  /* STRING */
  my_string string_field;
  
  /* STRUCT */
  my_struct struct_field;
  
  /* POINTER */
  my_pointer pointer_field;
  
  /* ARRAY */
  my_array array_field;
  
  /* CALLBACK */
  my_callback callback_field;
  
  /* UNION */
  my_union union_field;
};

/* Chain of structures for testing traversal */
struct GTY((chain_next = "%h.next", chain_prev = "%p.prev")) chain_struct {
  int id;
  my_string name;
  struct chain_struct *GTY((skip)) next;
  struct chain_struct *GTY((skip)) prev;
};

#endif /* TEST_GENGTYPE_TYPES_H */
