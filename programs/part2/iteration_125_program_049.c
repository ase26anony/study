/* test-gengtype-types.h - Test types for gengtype coverage testing */
#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

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

/* TYPE_STRING: String pointer typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2;
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY(()) my_struct_1 {
  int field1;
  double field2;
  const char *field3;
};

struct GTY(()) my_struct_2 {
  my_scalar_1 s_field;
  struct my_struct_1 *next;
  my_string_1 name;
};

struct GTY(()) my_struct_3 {
  int data[5];
  struct my_struct_2 *link;
  unsigned flags;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) user_struct_2 {
  double user_value;
  const char *user_name;
};

struct GTY((user)) user_struct_3 {
  long id;
  struct user_struct_1 *related;
};

/* TYPE_UNION: Union definitions */
union GTY((desc("%0.kind"))) my_union_1 {
  int int_val;
  double double_val;
  const char *string_val;
  struct my_struct_1 *struct_ptr;
};

union GTY((desc("%0.type"))) my_union_2 {
  my_scalar_1 scalar;
  my_string_1 string;
  struct my_struct_2 *sptr;
};

union GTY(()) my_union_3 {
  int data[4];
  struct {
    int a, b;
  } pair;
  void *generic;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef struct my_struct_2 *my_pointer_2;
typedef union my_union_1 *my_pointer_3;
typedef my_string_1 *string_ptr;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef double my_array_2[5][5];
typedef struct my_struct_1 *my_array_3[8];
typedef const char *string_array[20];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (*my_callback_1)(int, double);
typedef int (*my_callback_2)(const char *, struct my_struct_1 *);
typedef struct my_struct_2 *(*my_callback_3)(my_scalar_1, my_string_1);
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int kind;
  struct lang_struct_type_1 *next;
  union my_union_1 data;
};

struct GTY((desc("%0.tag"), chain_next="%h.link")) lang_struct_type_2 {
  enum { TAG_A, TAG_B, TAG_C } tag;
  struct lang_struct_type_2 *link;
  my_callback_1 callback;
};

struct GTY((desc("%d"), skip)) lang_struct_type_3 {
  int discriminator;
  struct lang_struct_type_1 *chain;
  my_array_1 buffer;
};

/* Additional struct with nested types to ensure traversal */
struct GTY(()) container_struct {
  struct my_struct_1 embedded_struct;
  union my_union_2 embedded_union;
  my_array_1 fixed_array;
  my_callback_2 callback_field;
  struct lang_struct_type_1 *lang_chain;
};

/* Union containing various pointer types */
union GTY((desc("%0.selector"))) pointer_union {
  int selector;
  struct my_struct_1 *struct_ptr;
  struct my_struct_2 *struct2_ptr;
  union my_union_1 *union_ptr;
  my_callback_1 func_ptr;
  my_string_1 string_ptr;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
