/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure gengtype processes each category. */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_scalar2;
typedef long my_scalar3;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef const char *my_string2;
typedef const char *my_string3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY(()) my_struct2 {
  double d;
  float f;
  char c;
};

struct GTY(()) my_struct3 {
  struct my_struct *next;
  int data;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct {
  void *data;
  int tag;
};

struct GTY((user)) my_user_struct2 {
  long id;
  void *ptr;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  float f;
  char *str;
};

union GTY(()) my_union2 {
  double d;
  long l;
  void *p;
};

union GTY(()) my_union3 {
  struct my_struct *s;
  union my_union *u;
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct *my_pointer;
typedef union my_union *my_pointer2;
typedef my_user_struct *my_pointer3;
typedef int *int_ptr;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef struct my_struct my_struct_array[5];
typedef const char *string_array[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*my_callback2)(const char *, int);
typedef struct my_struct *(*my_callback3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int type;
  struct lang_struct_type *next;
};

struct GTY((desc("%1"), tag("LANG_TYPE2"), skip(""))) lang_struct_type2 {
  int kind;
  void *data;
  struct lang_struct_type2 *chain;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with various field types */
struct GTY(()) complex_type {
  my_scalar scalar_field;
  my_string string_field;
  struct my_struct *struct_ptr;
  union my_union union_field;
  my_array array_field;
  my_callback callback_field;
  struct lang_struct_type *lang_field;
};

/* Struct with skip annotation */
struct GTY((skip)) skipped_struct {
  int x;
  int y;
};

/* Struct with maybe_undef annotation */
struct GTY((maybe_undef)) maybe_undefined_struct {
  int value;
  struct maybe_undefined_struct *next;
};

/* Chain of structures */
struct GTY((chain_next("%h.next"))) chain_struct {
  int id;
  struct chain_struct *next;
};

/* Variable length array in struct */
struct GTY(()) var_struct {
  int length;
  int data[1];
};

#endif /* TEST_GENGTYPE_TYPES_H */
