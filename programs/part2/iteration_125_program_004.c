/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_scalar2;
typedef long my_scalar3;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef const char *another_string;
typedef const char *gcc_string;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  my_scalar field1;
  my_string field2;
  int field3;
};

struct GTY(()) another_struct {
  my_scalar *ptr_field;
  my_string name;
  struct my_struct *next;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_struct {
  int value;
  struct linked_struct *next;
  struct linked_struct *prev;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_type {
  int user_data;
  void *user_pointer;
};

struct GTY((user)) another_user_struct {
  long id;
  const char *tag;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int int_val;
  my_scalar scalar_val;
  my_string string_val;
};

union GTY(()) another_union {
  long long_val;
  double double_val;
  struct my_struct *struct_ptr;
};

union GTY((desc("%0.int_val"))) tagged_union {
  int int_val;
  float float_val;
  const char *str_val;
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef my_scalar *scalar_pointer;
typedef const char *const *string_pointer_ptr;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef struct my_struct struct_array[5];
typedef my_string string_array[3];
typedef int multi_dim_array[2][3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(my_string, my_scalar);
typedef struct my_struct *(*struct_callback)(void);
typedef void (*complex_callback)(my_callback, another_callback);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("TREE_CODE (%1)"), skip)) tree_lang_struct {
  int code;
  union my_union *data;
};

struct GTY((desc("%0.type"), tag("TYPE"))) type_lang_struct {
  int type;
  struct type_lang_struct *next;
};

/* Additional types to ensure multiple instances */
struct GTY(()) extra_struct_one {
  my_array arr;
  my_callback cb;
};

struct GTY(()) extra_struct_two {
  union another_union u;
  lang_struct_type *lang_ptr;
};

/* Pointer to undefined type (should still be processed) */
struct undefined_type *GTY((skip)) undefined_pointer;

/* Array of pointers */
struct my_struct *GTY(()) struct_ptr_array[8];

/* Callback in struct */
struct GTY(()) struct_with_callback {
  my_callback handler;
  another_callback validator;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
