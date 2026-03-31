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

/* TYPE_STRING: String pointer types */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_1 * GTY((skip)) field3;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%1.type"), param_is("%1"))) my_struct_3 {
  int type;
  union {
    my_scalar_1 scalar_val;
    my_string_1 string_val;
    struct my_struct_1 *struct_val;
  } GTY((desc("%1.type"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct_1 {
  int user_field1;
  void *user_field2;
};

struct GTY((user)) my_user_struct_2 {
  double user_data;
  struct my_user_struct_1 *next_user;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  my_scalar_1 as_scalar;
  my_string_1 as_string;
  struct my_struct_1 *as_struct;
};

union GTY((desc("%1.utype"))) my_union_2 {
  int utype;
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
typedef struct my_struct_1 *my_pointer_1;
typedef struct my_struct_2 * GTY((skip)) my_pointer_2;
typedef union my_union_1 *my_pointer_3;

/* TYPE_ARRAY: Array types */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length("strlen(%h)"))) my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, my_scalar_1);
typedef struct my_struct_1 *(*my_callback_3)(my_scalar_2, my_string_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.lang_type"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_type;
  my_string_1 lang_name;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%1.kind"), param_is("%1"))) lang_struct_type_2 {
  enum { LANG_KIND_A, LANG_KIND_B, LANG_KIND_C } kind;
  union {
    my_scalar_1 scalar_data;
    my_string_1 string_data;
  } GTY((desc("%1.kind"))) data;
  struct lang_struct_type_2 * GTY((skip)) sibling;
};

/* Additional complex type to ensure thorough traversal */
struct GTY((tag("COMPLEX_TYPE"))) complex_container {
  /* Mix of all types */
  my_scalar_1 scalar_field;
  my_string_1 string_field GTY((length("strlen(%h.string_field)")));
  struct my_struct_1 *struct_field;
  struct my_user_struct_1 *user_field GTY((skip));
  union my_union_1 union_field;
  my_array_1 array_field;
  my_callback_1 callback_field;
  struct lang_struct_type_1 *lang_field;
  
  /* Nested structures */
  struct {
    int nested_scalar;
    my_string_1 nested_string;
  } GTY((tag("NESTED"))) nested;
  
  /* Pointer array */
  struct my_struct_2 * GTY((length("%h.array_len"))) pointer_array[10];
  int array_len;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
