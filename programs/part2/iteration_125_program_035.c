/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure gengtype processes each category. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef char *my_mutable_string;
typedef const char * const my_const_string;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY((skip)) another_struct {
  double d;
  float f;
};

struct GTY((chain_next = "%h.next")) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_type {
  void *data;
  int tag;
};

struct GTY((user)) another_user_struct {
  long id;
  char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  float f;
  double d;
};

union GTY((desc("%0"))) tagged_union {
  int type;
  struct my_struct s;
  union my_union u;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef user_struct_type *user_struct_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(my_struct *, my_string);
typedef void (*void_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("%0"), tag("LANG_TYPE"))) another_lang_struct {
  enum { LANG_A, LANG_B } tag;
  union {
    int a;
    float b;
  } u;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with various field types */
struct GTY(()) complex_nested {
  my_scalar scalar_field;
  my_string string_field;
  my_array array_field;
  my_callback callback_field;
  struct my_struct struct_field;
  union my_union union_field;
  my_pointer pointer_field;
};

/* Struct with array of pointers */
struct GTY(()) pointer_array_struct {
  my_pointer pointers[5];
  my_callback callbacks[3];
};

/* Union with struct and array */
union GTY((desc("%0"))) mixed_union {
  int discriminant;
  struct my_struct s;
  my_array a;
  my_pointer p;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
