/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

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
typedef char *my_string_2;
typedef const char * const my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) test_struct_1 {
  int field1;
  double field2;
  const char *field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) test_struct_2 {
  struct test_struct_2 *next;
  struct test_struct_2 *prev;
  my_scalar_1 data;
};

struct GTY((skip)) test_struct_3 {
  void *ptr;
  int count;
  my_string_1 name;
};

/* TYPE_USER_STRUCT: Structs with user tag */
struct GTY((user)) user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) user_struct_2 {
  double value;
  const char *description;
};

struct GTY((user)) user_struct_3 {
  long id;
  struct user_struct_3 *link;
};

/* TYPE_UNION: Union types */
union GTY(()) test_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc ("%1.type"))) test_union_2 {
  struct {
    int type;
    union test_union_2 *next;
  } header;
  int int_value;
  double double_value;
  const char *string_value;
};

union GTY(()) test_union_3 {
  my_scalar_1 scalar;
  my_string_2 string;
  struct test_struct_1 *struct_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct test_struct_1 *my_pointer_1;
typedef union test_union_1 *my_pointer_2;
typedef const my_scalar_2 *my_pointer_3;
typedef struct undefined_type_1 *my_pointer_4;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct test_struct_1 my_array_2[5];
typedef const char *my_array_3[20];
typedef union test_union_2 my_array_4[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, double);
typedef int (*my_callback_2)(const char *, void *);
typedef struct test_struct_1 *(*my_callback_3)(my_scalar_1, my_string_1);
typedef void (*my_callback_4)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.tag"), chain_next="%h.next")) lang_struct_1 {
  enum tree_code tag;
  struct lang_struct_1 *next;
  union tree_node *gt_ggc_rt;
};

struct GTY((desc("%0.type"), tag("1"))) lang_struct_2 {
  int type;
  struct lang_struct_2 *left;
  struct lang_struct_2 *right;
  const char *name;
};

struct GTY((desc("%1.kind"), chain_prev="%h.prev")) lang_struct_3 {
  enum gimple_code kind;
  struct lang_struct_3 *prev;
  struct lang_struct_3 *next;
  location_t loc;
};

/* Additional structs to ensure multiple counts */
struct GTY(()) extra_struct_1 { int a; double b; };
struct GTY(()) extra_struct_2 { char *name; int id; };
struct GTY(()) extra_struct_3 { void *data; size_t size; };

union GTY(()) extra_union_1 { int x; long y; };
union GTY(()) extra_union_2 { float f; double d; };
union GTY(()) extra_union_3 { const char *s; int i; };

/* More pointers for coverage */
typedef extra_struct_1 *extra_ptr_1;
typedef extra_union_2 *extra_ptr_2;
typedef my_array_1 *extra_ptr_3;

/* More arrays for coverage */
typedef extra_struct_2 extra_array_1[8];
typedef my_callback_1 extra_array_2[4];
typedef void *extra_array_3[15];

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
