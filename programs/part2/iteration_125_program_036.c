/* test-gengtype-types.h - Test types for gengtype coverage */
/* This file defines types corresponding to each type_kind enum case */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int test_scalar_1;
typedef long test_scalar_2;
typedef unsigned char test_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *test_string_1;
typedef const char *test_string_2 GTY(());
typedef const char *test_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY(()) test_struct_1 {
  test_scalar_1 field1;
  test_string_1 field2;
  struct undefined_type_1 *field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) test_struct_2 {
  test_scalar_2 field1;
  test_string_2 field2;
  struct test_struct_1 *next;
  struct test_struct_1 *prev;
};

struct GTY((skip)) test_struct_3 {
  test_scalar_3 field1;
  test_string_3 field2;
  void *private_data;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) test_user_struct_1 {
  int user_field1;
  void *user_field2;
};

struct GTY((user)) test_user_struct_2 {
  long user_field1;
  const char *user_field2;
};

/* TYPE_UNION: Union definitions */
union GTY(()) test_union_1 {
  test_scalar_1 as_scalar;
  test_string_1 as_string;
  struct test_struct_1 *as_struct;
};

union GTY((desc("%0.type"))) test_union_2 {
  int type;
  struct test_struct_2 *ptr;
  test_scalar_2 value;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct test_struct_1 *test_pointer_1;
typedef union test_union_1 * GTY(()) test_pointer_2;
typedef test_scalar_1 *test_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int test_array_1[10];
typedef struct test_struct_1 test_array_2[5] GTY(());
typedef const char *test_array_3[20];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (*test_callback_1)(int);
typedef int (*test_callback_2)(const char *, void *) GTY(());
typedef void (*test_callback_3)(struct test_struct_1 *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) test_lang_struct_1 {
  int tag;
  union {
    test_scalar_1 scalar_val;
    test_string_1 string_val;
  } u;
  struct test_lang_struct_1 *next;
};

struct GTY((desc("%0.kind"), tag("0"))) test_lang_struct_2 {
  enum { KIND_A, KIND_B, KIND_C } kind;
  union {
    struct test_struct_1 *a;
    test_scalar_2 b;
    test_string_2 c;
  } data;
};

/* Additional composite types to ensure coverage */
struct GTY(()) composite_struct {
  test_array_1 arr_field;
  test_pointer_1 ptr_field;
  test_callback_1 callback_field;
  union test_union_2 union_field;
};

/* Nested structure with multiple pointer types */
struct GTY(()) nested_struct {
  struct test_struct_1 *first;
  struct test_struct_2 *second;
  test_pointer_3 third;
  test_array_2 array_field;
};

#endif /* TEST_GENGTYPE_TYPES_H */
