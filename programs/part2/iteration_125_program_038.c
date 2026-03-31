/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure gengtype processes each category. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String pointer typedefs */
typedef const char *my_string;
typedef char *my_mutable_string;
typedef const char *const my_const_string;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY(()) my_struct {
  my_scalar field1;
  my_string field2;
  int field3;
};

struct GTY(()) another_struct {
  double d;
  float f;
  char c;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
  struct linked_struct *prev;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) user_struct_type {
  int user_data;
  void *user_pointer;
};

struct GTY((user)) another_user_struct {
  long id;
  const char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  double d;
  my_string s;
};

union GTY(()) another_union {
  long l;
  float f;
  char *str;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct *my_pointer;
typedef another_struct *another_pointer;
typedef my_union *union_pointer;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(my_string, my_scalar);
typedef void (*complex_callback)(my_struct *, my_array);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_id;
  void *lang_data;
  struct lang_struct_type *next;
};

struct GTY((desc("%0"), tag("1"))) another_lang_struct {
  enum { LANG_A, LANG_B } tag;
  union {
    int a;
    double b;
  } GTY((desc("%1.tag"))) u;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  my_scalar scalar_field;
  my_string string_field;
  my_struct struct_field;
  my_union union_field;
  my_pointer pointer_field;
  my_array array_field;
  my_callback callback_field;
  struct lang_struct_type *lang_field;
};

/* Nested struct with various type combinations */
struct GTY(()) nested_types {
  /* Scalar */
  int count;
  
  /* String */
  const char *name;
  
  /* Struct */
  struct inner_struct {
    int x;
    int y;
  } GTY(()) inner;
  
  /* Union */
  union {
    int as_int;
    float as_float;
  } value;
  
  /* Pointer */
  struct nested_types *next;
  
  /* Array */
  int numbers[5];
  
  /* Callback */
  void (*handler)(struct nested_types *);
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
