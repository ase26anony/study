/* test-gengtype-types.h - Test types for gengtype coverage */
/* This file defines types covering all enum type_kind cases in gengtype.cc */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length ("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((tag ("STRUCT_1"))) my_struct_1 {
  int field1;
  double field2;
  const char * GTY((skip)) field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) my_struct_2 {
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
  my_scalar_1 value;
};

struct GTY((desc ("%0.kind"))) my_struct_3 {
  enum { KIND_A, KIND_B, KIND_C } kind;
  union {
    int int_val;
    double double_val;
    const char *string_val;
  } GTY((desc ("%1.kind"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) my_user_struct_2 {
  long custom_field;
  struct undefined_type_1 *forward_ref;
};

/* TYPE_UNION: Union types */
union GTY((tag ("UNION_1"))) my_union_1 {
  int int_val;
  double double_val;
  void *ptr_val;
};

union GTY((desc ("%0.type"))) my_union_2 {
  int type;
  struct {
    int a;
    int b;
  } pair;
  struct {
    double x;
    double y;
    double z;
  } point;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length ("strlen(%h[i])"))) my_string_array[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, double);
typedef int (*my_callback_2)(const char *, size_t);
typedef void (* GTY((skip)) my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc ("%1.kind"), chain_next ("%h.next"))) lang_struct_type_1 {
  enum lang_kind { LANG_A, LANG_B } kind;
  struct lang_struct_type_1 *next;
  union {
    int int_val;
    const char *str_val;
  } GTY((desc ("%1.kind"))) data;
};

struct GTY((desc ("%0.type"), tag ("LANG_STRUCT_2"))) lang_struct_type_2 {
  int type;
  struct my_struct_1 *first;
  struct my_struct_2 *second;
  my_callback_1 callback;
};

/* Additional struct to ensure multiple instances */
struct GTY(()) another_struct {
  my_array_1 arr;
  my_pointer_1 ptr;
  my_string_1 str;
};

/* Another union for multiple instances */
union GTY((tag ("ANOTHER_UNION"))) another_union {
  struct my_struct_1 s;
  union my_union_1 u;
  my_scalar_1 scalar;
};

#endif /* TEST_GENGTYPE_TYPES_H */
