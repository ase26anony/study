/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String types */
typedef const char *my_string_1;
typedef const char * GTY(()) my_string_2;
typedef const char * GTY((skip)) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((desc("%1"))) my_struct_2 {
  int a;
  double b;
  struct my_struct_1 *next;
};

struct GTY((chain_next = "%h.next")) my_struct_3 {
  int value;
  struct my_struct_3 *next;
  my_string_2 name;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct_1 {
  int user_data;
  void *user_pointer;
};

struct GTY((user)) my_user_struct_2 {
  double x;
  double y;
  struct my_user_struct_1 *related;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  double as_double;
  void *as_pointer;
};

union GTY((desc("%d"))) my_union_2 {
  long long big;
  struct {
    int part1;
    int part2;
  } parts;
};

union GTY((skip)) my_union_3 {
  char bytes[8];
  int ints[2];
  float floats[2];
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY(()) my_pointer_2;
typedef my_user_struct_1 * GTY((skip)) my_pointer_3;

/* TYPE_ARRAY: Array types */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef union my_union_2 GTY(()) my_array_3[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, void *);
typedef void (* GTY(()) my_callback_3)(struct my_struct_1 *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type_1 {
  int lang_specific;
  struct lang_struct_type_1 *next;
  my_callback_1 handler;
};

struct GTY((desc("%d"), tag("LANG"))) lang_struct_type_2 {
  enum { LANG_TYPE_A, LANG_TYPE_B } type;
  union {
    int int_val;
    double double_val;
    my_string_1 str_val;
  } data;
};

struct GTY((desc("%1"), skip)) lang_struct_type_3 {
  int id;
  my_array_1 buffer;
  my_pointer_2 ptr;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  struct my_struct_1 *struct_ptr;
  union my_union_2 current_union;
  my_array_2 struct_array;
  my_callback_2 callback;
  struct lang_struct_type_1 *lang_chain;
};

/* Nested type definitions */
typedef struct GTY(()) {
  int nested_field;
  my_pointer_1 nested_ptr;
} nested_struct_type;

/* Another user struct with different configuration */
struct GTY((user)) final_user_struct {
  nested_struct_type nested;
  my_array_3 union_array;
  my_callback_3 final_callback;
};

#endif /* TEST_GENGTYPE_TYPES_H */
