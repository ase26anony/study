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
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String pointer typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2 GTY(());
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((skip)) my_struct_3 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_scalar_3 field3;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  int user_field1;
  void *user_field2;
};

struct GTY((user)) my_user_struct_2 {
  double user_field1;
  const char *user_field2;
};

struct GTY((user)) my_user_struct_3 {
  long user_field1;
  int user_field2;
  void *user_field3;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  my_scalar_1 as_scalar;
  my_string_1 as_string;
  struct my_struct_1 *as_struct;
};

union GTY((desc ("%1.type"))) my_union_2 {
  int type;
  struct {
    int type;
    my_scalar_2 value;
  } scalar_data;
  struct {
    int type;
    my_string_2 value;
  } string_data;
};

union GTY(()) my_union_3 {
  double d;
  float f;
  int i;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 *my_pointer_2 GTY(());
typedef my_user_struct_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5] GTY(());
typedef const char *my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, double) GTY(());
typedef void (*my_callback_3)(struct my_struct_1 *, union my_union_1 *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc ("%1.kind"), tag ("LANG_STRUCT_KIND"))) lang_struct_1 {
  int kind;
  union {
    my_scalar_1 scalar_val;
    my_string_1 string_val;
  } u;
};

struct GTY((desc ("%1.type"), chain_next ("%h.next"))) lang_struct_2 {
  int type;
  my_array_1 data;
  struct lang_struct_2 *next;
};

struct GTY((desc ("%0.type"), skip)) lang_struct_3 {
  int type;
  my_callback_1 callback;
  my_pointer_1 ptr;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  my_struct_1 struct_field;
  my_union_2 union_field;
  my_array_2 array_field;
  my_pointer_3 pointer_field;
  my_callback_2 callback_field;
  struct lang_struct_1 *lang_field;
};

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_field;
    my_string_3 str_field;
  } nested;
  
  union GTY(()) inner_union {
    int i;
    double d;
  } data;
  
  my_array_3 string_array;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
