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

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2 GTY(());
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((skip)) my_struct_3 {
  int private_field1;
  double private_field2;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct_1 {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) my_user_struct_2 {
  long custom_handle;
  const char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  my_scalar_1 as_int;
  my_scalar_3 as_double;
  my_string_1 as_string;
};

union GTY((desc("%0.type"))) my_union_2 {
  int type;
  struct my_struct_1 *as_struct;
  union my_union_1 *as_union;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY(()) my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY(()) my_array_3[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, double);
typedef void (* GTY(()) my_callback_3)(struct my_struct_1 *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int discriminator;
  union my_union_2 data;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0.kind"), tag("0"))) lang_struct_type_2 {
  enum { KIND_A, KIND_B } kind;
  union {
    struct my_struct_1 * GTY((tag("0"))) a;
    struct my_struct_2 * GTY((tag("1"))) b;
  } u;
};

/* Additional types to ensure multiple instances */
struct GTY(()) extra_struct_1 {
  my_array_1 arr;
  my_callback_1 cb;
};

union GTY(()) extra_union_1 {
  my_pointer_1 ptr;
  my_scalar_2 num;
};

typedef extra_struct_1 *extra_pointer_1;
typedef extra_union_1 extra_array_1[8];

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
