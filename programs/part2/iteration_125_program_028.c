/* test-gengtype-types.h - Test types for gengtype coverage */
/* This file defines types corresponding to each type_kind enum case */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int test_scalar_1;
typedef long test_scalar_2;
typedef unsigned char test_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *test_string_1;
typedef const char *test_string_2 GTY((skip));
typedef const char *test_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((desc("%0"))) test_struct_1 {
  test_scalar_1 field1;
  test_string_1 field2;
  struct test_struct_1 *next;
};

struct GTY((chain_next("%h.next"))) test_struct_2 {
  int a;
  double b;
  struct test_struct_2 *next;
};

struct GTY((desc("%d"))) test_struct_3 {
  test_scalar_2 id;
  test_string_2 name;
  struct test_struct_3 *prev;
  struct test_struct_3 *next;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) test_user_struct_1 {
  void *opaque_data;
};

struct GTY((user)) test_user_struct_2 {
  int user_id;
  void *user_ptr;
};

/* TYPE_UNION: Union types */
union GTY((desc("%0"))) test_union_1 {
  test_scalar_1 as_int;
  test_string_1 as_string;
  void *as_ptr;
};

union GTY((tag("union_type"))) test_union_2 {
  int i;
  double d;
  char *s;
};

union test_union_3 {
  long l;
  float f;
  struct test_struct_1 *p;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct test_struct_1 *test_pointer_1;
typedef union test_union_1 *test_pointer_2 GTY((skip));
typedef test_scalar_1 *test_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int test_array_1[10];
typedef struct test_struct_1 test_array_2[5];
typedef char test_array_3[256];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*test_callback_1)(int);
typedef int (*test_callback_2)(const char *, void *);
typedef void (*test_callback_3)(struct test_struct_1 *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_id;
  void *lang_data;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0"), chain_prev="%p.prev", chain_next="%n.next")) lang_struct_type_2 {
  int type_code;
  const char *type_name;
  struct lang_struct_type_2 *prev;
  struct lang_struct_type_2 *next;
};

/* Additional mixed types to ensure coverage */
struct GTY(()) complex_type {
  test_scalar_1 id;
  test_string_1 name;
  test_array_1 buffer;
  test_callback_1 callback;
  union test_union_1 data;
  struct test_struct_1 *link;
};

/* Pointer to undefined type (should still be processed) */
struct undefined_type_1 *undefined_ptr_1;
struct undefined_type_2 *undefined_ptr_2;

#endif /* TEST_GENGTYPE_TYPES_H */
