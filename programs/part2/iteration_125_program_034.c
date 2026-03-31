/* test-gengtype-types.h - Test types for gengtype coverage testing */
/* This file defines types corresponding to all type_kind enum cases */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

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
typedef const char *my_string_3 GTY((skip));

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((desc("%1"))) my_struct_2 {
  int a;
  double b;
  struct my_struct_1 *next GTY((tag("0")));
};

struct GTY((chain_next("%h.next"))) my_struct_3 {
  int value;
  struct my_struct_3 *next;
  my_string_2 name;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  int user_field1;
  void *user_field2;
};

struct GTY((user)) my_user_struct_2 {
  double data;
  struct my_user_struct_1 *related;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  double as_double;
  const char *as_string;
};

union GTY((desc("%0"))) my_union_2 {
  struct my_struct_1 *s;
  union my_union_1 *u;
  my_scalar_3 scalar;
};

union my_union_3 {
  long long_val;
  void *ptr_val;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_2 *my_pointer_2 GTY(());
typedef my_scalar_2 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5] GTY(());
typedef const char *my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, double) GTY(());
typedef struct my_struct_1 *(*my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_id;
  struct lang_struct_type_1 *next;
  my_callback_1 handler;
};

struct GTY((desc("%0"), tag("1"))) lang_struct_type_2 {
  unsigned flags;
  my_array_1 data;
  union my_union_3 variant;
};

struct GTY((desc("%1"), skip)) lang_struct_type_3 {
  double precision;
  my_string_3 message;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  struct my_struct_1 struct_field;
  union my_union_2 union_field;
  my_array_1 array_field;
  my_callback_2 callback_field;
  struct lang_struct_type_1 *lang_field;
};

typedef struct complex_container *container_ptr GTY(());

#endif /* TEST_GENGTYPE_TYPES_H */
