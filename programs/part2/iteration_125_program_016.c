/* Test types for gengtype coverage testing.
   This file defines types corresponding to each enum type_kind case
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
typedef const char * GTY((length("strlen(%h)+1"))) my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_1 * GTY((skip)) next;
  struct my_struct_1 *prev;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  int type;
  union {
    my_scalar_1 scalar_val;
    my_string_3 string_val;
  } GTY((desc("%1.type"))) value;
};

/* TYPE_USER_STRUCT: Structs with user tag */
struct GTY((user)) my_user_struct_1 {
  void *user_data;
  int user_id;
};

struct GTY((user)) my_user_struct_2 {
  long custom_field;
  struct my_user_struct_1 *related;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  my_scalar_1 as_scalar;
  my_string_1 as_string;
  struct my_struct_1 *as_struct;
};

union GTY((desc("%1.utype"))) my_union_2 {
  int utype;
  my_scalar_2 scalar_data;
  my_string_2 string_data;
  struct my_struct_2 *struct_data;
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct_1 *my_pointer_1;
typedef struct my_struct_2 * GTY((skip)) my_pointer_2;
typedef union my_union_1 *my_pointer_3;

/* TYPE_ARRAY: Array types */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length("strlen(%h[i])+1"))) my_array_3[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, my_scalar_1);
typedef struct my_struct_1 *(*my_callback_3)(my_pointer_1, my_array_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.lang_type"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_type;
  my_string_1 lang_name;
  struct lang_struct_type_1 *next;
  my_callback_1 callback;
};

struct GTY((desc("%1.kind"), tag("LANG_STRUCT_2"))) lang_struct_type_2 {
  enum { LANG_KIND_1, LANG_KIND_2 } kind;
  union {
    my_scalar_1 scalar_field;
    my_string_2 string_field;
  } GTY((desc("%1.kind"))) data;
  my_array_1 fixed_array;
};

/* Additional complex nested types to ensure thorough traversal */
struct GTY((tag("COMPLEX_STRUCT"))) complex_nested {
  my_pointer_1 ptr_field;
  my_array_2 array_field;
  union my_union_2 union_field;
  my_callback_2 callback_field;
  struct lang_struct_type_1 * GTY((chain_next("%h.lang_next"))) lang_next;
};

/* TYPE_NONE should not be directly definable - it's used internally */

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
