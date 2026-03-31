/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   in gengtype.cc to ensure all switch cases are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_scalar2;
typedef long my_scalar3;

/* TYPE_STRING: String typedefs */
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
  double d;
  void *p;
};

union GTY(()) my_union2 {
  char c[4];
  int i;
  float f;
};

union GTY(()) my_union3 {
  struct my_struct *s;
  union my_union *u;
  int i;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct *my_pointer;
typedef my_union *my_pointer2;
typedef my_user_struct *my_pointer3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array[10];
typedef my_struct *my_array2[5];
typedef const char *my_string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*my_callback2)(const char *, void *);
typedef void (*my_callback3)(struct my_struct *, union my_union *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int tag;
  struct lang_struct_type *next;
  union my_union data;
};

struct GTY((desc("%0"), tag("TAG_A", "TAG_B"))) lang_struct_type2 {
  int tag;
  struct lang_struct_type2 *next;
  void *data;
};

struct GTY((desc("%1"), skip)) lang_struct_type3 {
  int type;
  struct lang_struct_type3 *chain;
  my_callback func;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  my_array arr;
  my_pointer ptr;
  my_callback cb;
  union my_union u;
};

/* Nested struct with various type kinds */
struct GTY(()) nested_types {
  /* scalar */
  int count;
  
  /* string */
  const char *name;
  
  /* pointer */
  struct complex_container *container;
  
  /* array */
  my_callback callbacks[5];
  
  /* union */
  union {
    int i;
    double d;
  } value;
  
  /* callback */
  my_callback handler;
};

/* Struct with chain_next for linked list testing */
struct GTY((chain_next("%h.next"))) linked_node {
  int data;
  struct linked_node *next;
  struct linked_node *prev;
};

/* Struct with length annotation for variable-sized array */
struct GTY((length("%h.len"))) var_array_struct {
  int len;
  int data[1];
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
