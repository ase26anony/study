/* Test types for gengtype coverage - covering all type_kind cases */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef const char *another_string_type;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) test_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_struct {
  int data;
  struct linked_struct *GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc ("%1.type"))) tagged_struct {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_val;
    my_string str_val;
  } GTY((desc ("%0.type"))) value;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_defined_struct {
  int user_data;
  void *user_pointer;
};

struct GTY((user)) another_user_struct {
  double user_double;
  int user_array[5];
};

/* TYPE_UNION: Union types */
union GTY(()) test_union {
  int int_member;
  double double_member;
  my_string string_member;
};

union GTY((desc ("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    int width;
    int height;
  } rect;
};

/* TYPE_POINTER: Pointer types */
typedef struct test_struct *struct_pointer;
typedef union test_union *union_pointer;
typedef my_scalar *scalar_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef struct test_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(struct test_struct *, my_string);
typedef void (*void_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("%1.tag"), chain_prev="%h.prev")) another_lang_struct {
  enum lang_tag { LANG_TAG_A, LANG_TAG_B } tag;
  struct another_lang_struct *prev;
  union {
    int int_val;
    double double_val;
  } data;
};

/* Additional types to ensure multiple instances */

/* More structs for TYPE_STRUCT */
struct GTY(()) extra_struct_one {
  my_array array_field;
  simple_callback callback_field;
};

struct GTY((skip)) extra_struct_two {
  union_pointer union_ptr;
  scalar_pointer scalar_ptr;
};

/* More unions for TYPE_UNION */
union GTY(()) extra_union {
  struct test_struct *s_ptr;
  struct linked_struct *l_ptr;
};

/* More pointers for TYPE_POINTER */
typedef extra_struct_one *extra_ptr_one;
typedef extra_struct_two *extra_ptr_two;

/* More arrays for TYPE_ARRAY */
typedef simple_callback callback_array[4];
typedef int multi_dim_array[3][4];

/* More callbacks for TYPE_CALLBACK */
typedef int (*filter_callback)(int *, size_t);
typedef void (*error_callback)(const char *, ...);

#endif /* TEST_GENGTYPE_TYPES_H */
