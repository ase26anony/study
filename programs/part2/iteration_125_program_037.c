/* Test types for gengtype coverage testing.
   This file defines types corresponding to each enum type_kind case
   in gengtype.cc to ensure all switch cases are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_2 *next;
};

struct GTY((desc("%1.tag"))) my_struct_3 {
  int tag;
  union {
    my_scalar_1 scalar_val;
    my_string_3 string_val;
  } GTY((desc("%1.tag"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  void *opaque_data;
  int user_field;
};

struct GTY((user)) my_user_struct_2 {
  long id;
  struct my_user_struct_2 *next;
};

/* TYPE_UNION: Union types */
union GTY((desc("%1.type"))) my_union_1 {
  my_scalar_1 as_scalar;
  my_string_1 as_string;
  struct my_struct_1 *as_struct;
};

union GTY(()) my_union_2 {
  int int_val;
  double double_val;
  void *ptr_val;
};

union GTY((skip)) my_union_3 {
  my_scalar_2 scalar_field;
  my_string_2 string_field;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_user_struct_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef union my_union_2 GTY((length("10"))) my_array_3[10];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, void *);
typedef struct my_struct_1 *(*my_callback_3)(int, my_string_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.kind"), chain_next="%h.next")) lang_struct_type_1 {
  enum { LANG_1, LANG_2, LANG_3 } kind;
  union {
    my_scalar_1 scalar_data;
    my_string_1 string_data;
  } GTY((desc("%1.kind"))) data;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("0"), tag("1"))) lang_struct_type_2 {
  int lang_id;
  my_callback_1 callback;
  my_array_1 buffer;
};

struct GTY((desc("%1.type"), chain_prev="%p.prev", chain_next="%h.next")) lang_struct_type_3 {
  int type;
  my_pointer_1 data;
  struct lang_struct_type_3 *prev;
  struct lang_struct_type_3 *next;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with various field types */
struct GTY(()) complex_nested_struct {
  my_scalar_1 id;
  my_string_1 name;
  my_array_1 scores;
  my_pointer_1 ptr;
  union my_union_1 data;
  my_callback_1 handler;
  struct complex_nested_struct * GTY((skip)) sibling;
};

/* Struct containing all type kinds as fields */
struct GTY((desc("%1.magic"))) all_types_struct {
  int magic;  /* TYPE_SCALAR */
  const char * GTY((length("strlen(%h.str)"))) str;  /* TYPE_STRING */
  struct my_struct_1 nested;  /* TYPE_STRUCT */
  struct my_user_struct_1 *user;  /* TYPE_USER_STRUCT */
  union my_union_2 data;  /* TYPE_UNION */
  int *int_ptr;  /* TYPE_POINTER */
  double matrix[3][3];  /* TYPE_ARRAY */
  void (*func_ptr)(void);  /* TYPE_CALLBACK */
  struct lang_struct_type_1 *lang;  /* TYPE_LANG_STRUCT */
};

/* Forward declarations for circular references */
struct forward_decl_1;
struct forward_decl_2;

struct GTY(()) forward_decl_1 {
  int value;
  struct forward_decl_2 *partner;
};

struct GTY(()) forward_decl_2 {
  int counter;
  struct forward_decl_1 *partner;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
