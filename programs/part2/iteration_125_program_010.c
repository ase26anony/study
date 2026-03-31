/* Test types for gengtype coverage testing.
   This file defines types corresponding to each enum type_kind case
   in gengtype.cc to ensure all switch branches are executed. */

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

/* TYPE_STRING: String pointer typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2;
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) test_struct_1 {
  my_scalar_1 field1;
  my_string_1 field2;
  struct undefined_type_1 *field3;
};

struct GTY((chain_next ("%h.next"))) test_struct_2 {
  int data;
  struct test_struct_2 *next;
};

struct GTY((skip)) test_struct_3 {
  void *ptr;
  int count;
};

/* TYPE_USER_STRUCT: Structs with user tag */
struct GTY((user)) user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) user_struct_2 {
  double value;
  const char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) test_union_1 {
  int as_int;
  float as_float;
  void *as_ptr;
};

union GTY((desc ("%0.as_int"))) test_union_2 {
  int as_int;
  double as_double;
  struct test_struct_1 *as_struct;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct test_struct_1 *my_pointer_1;
typedef union test_union_1 *my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Fixed-size array typedefs */
typedef int my_array_1[10];
typedef struct test_struct_1 my_array_2[5];
typedef const char *my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, void *);
typedef struct test_struct_1 *(*my_callback_3)(int, int);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_id;
  void *lang_data;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0.lang_id"))) lang_struct_type_2 {
  int lang_id;
  union test_union_1 data;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  my_array_1 arr;
  my_callback_1 cb;
  union test_union_2 uni;
  struct test_struct_2 *list;
};

/* Nested pointer/array combinations */
typedef my_callback_2 callback_array[5];
typedef struct test_struct_1 *struct_ptr_array[3][4];

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
