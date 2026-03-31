/* Test types for gengtype coverage testing.
   This file defines types corresponding to each enum type_kind case
   in gengtype.cc to ensure all switch cases are executed. */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

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
typedef char *mutable_string GTY((skip));

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY((desc("%0"))) my_struct {
  my_scalar field1;
  my_string field2;
  int field3;
};

struct GTY((chain_next = "%h.next", chain_prev = "%h.prev")) linked_struct {
  int data;
  struct linked_struct * GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc("TREE_CODE (%h)"), tag("TREE"))) tree_struct {
  int code;
  union tree_node * GTY((skip)) chain;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_type {
  void *data;
  int user_tag;
};

struct GTY((user)) another_user_struct {
  long custom_data;
  void (*cleanup)(void*);
};

/* TYPE_UNION: Union types */
union GTY((desc("%1"))) my_union {
  int int_val;
  my_scalar scalar_val;
  my_string string_val;
};

union GTY((tag("UNION_TYPE"))) tagged_union {
  struct my_struct *struct_ptr;
  union my_union *union_ptr;
  void *generic_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef union my_union *union_pointer_type;
typedef user_struct_type * GTY((skip)) user_struct_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[20] GTY((length("strlen(%h)")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, const char*);
typedef int (*comparison_callback)(const void*, const void*);
typedef void (*cleanup_callback)(void*) GTY((skip));

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
  void * GTY((skip)) lang_data;
};

struct GTY((desc("LANG_TYPE"), tag("LANG"))) another_lang_struct {
  int language_id;
  struct another_lang_struct * GTY((chain_next = "%h.next")) chain;
  union tagged_union data;
};

/* Additional types to ensure multiple instances */
struct GTY(()) extra_struct_one {
  my_array array_field;
  my_callback callback_field;
};

struct GTY(()) extra_struct_two {
  union_pointer_type union_ptr;
  string_array strings;
};

/* Nested types for complex testing */
struct GTY((desc("NESTED"))) nested_container {
  struct my_struct inner_struct;
  union my_union inner_union;
  lang_struct_type *lang_ptr;
  my_callback handlers[3];
};

#endif /* TEST_GENGTYPE_TYPES_H */
