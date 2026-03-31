/* test-gengtype-types.h - Test types for gengtype coverage */
/* This file defines types covering all enum type_kind cases in gengtype.cc */

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

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2 GTY(());
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_string_1 field2;
  struct my_struct_1 *next;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  int x;
  double y;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((skip)) my_struct_3 {
  int a;
  int b;
  int c;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  int data;
  void *opaque;
};

struct GTY((user)) my_user_struct_2 {
  double value;
  const char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%0.as_int"))) my_union_2 {
  int type;
  struct my_struct_1 *s;
  my_scalar_1 n;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 *my_pointer_2 GTY(());
typedef my_scalar_2 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5] GTY(());
typedef const char *my_array_3[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(struct my_struct_1 *);
typedef void (*my_callback_3)(void) GTY(());

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_specific;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("0"), tag("1"))) lang_struct_type_2 {
  enum { LANG_TYPE_A, LANG_TYPE_B } kind;
  union {
    struct my_struct_1 *s;
    my_scalar_1 n;
  } u;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with multiple pointer types */
struct GTY(()) complex_type_1 {
  struct my_struct_1 *ptr1;
  union my_union_2 *ptr2;
  my_callback_1 callback;
  my_array_1 arr;
};

/* Union containing structs */
union GTY((desc("%0.tag"))) complex_type_2 {
  struct {
    int tag;
    int data;
  } header;
  struct {
    int tag;
    struct my_struct_1 *item;
    my_string_2 name;
  } node;
};

/* Struct with array of pointers */
struct GTY(()) complex_type_3 {
  struct my_struct_1 *items[5];
  my_callback_2 processor;
  union my_union_1 values[3];
};

#endif /* TEST_GENGTYPE_TYPES_H */
