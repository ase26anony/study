/* test-gengtype-types.h - Test types for gengtype coverage */
/* This file defines types corresponding to each type_kind enum case */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_scalar2;
typedef double my_scalar3;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef const char *another_string;
typedef const char *third_string;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  int field1;
  double field2;
  my_scalar field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_struct {
  int data;
  struct linked_struct * GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc("%1.type"))) tagged_struct {
  enum { TYPE_A, TYPE_B } type;
  union {
    int a;
    double b;
  } value;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *opaque_data;
  int user_id;
};

struct GTY((user)) another_user_struct {
  long custom_field;
  void *private_data;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int as_int;
  double as_double;
  void *as_pointer;
};

union GTY((desc("%0.kind"))) tagged_union {
  enum { KIND_INT, KIND_DOUBLE } kind;
  struct {
    int as_int;
    double as_double;
  } value;
};

union GTY((skip)) simple_union {
  int a;
  long b;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef user_struct *user_pointer;
typedef my_union *union_pointer;
typedef const char **string_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];
typedef int multi_dim_array[2][3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, const char*);
typedef int (*another_callback)(void);
typedef void (*complex_callback)(my_struct*, my_callback);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("%0.tag"), chain_prev="%h.prev")) another_lang_struct {
  enum lang_tag tag;
  struct another_lang_struct *prev;
  void * GTY((skip)) lang_data;
};

/* Additional mixed types to ensure coverage */
struct GTY(()) container_struct {
  my_scalar scalar_field;
  my_string string_field;
  my_pointer pointer_field;
  my_array array_field;
  my_callback callback_field;
  struct my_struct nested_struct;
  union my_union nested_union;
};

/* Pointer to undefined type (should still be processed) */
struct undefined_type *undefined_pointer;

#endif /* TEST_GENGTYPE_TYPES_H */
