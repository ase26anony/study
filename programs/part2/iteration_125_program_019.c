/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum value
   to ensure all switch cases in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined_type;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String typedefs */
typedef const char *my_string;
typedef char *my_mutable_string;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc ("%0.kind"))) tagged_struct {
  enum { KIND_A, KIND_B } kind;
  union {
    int a;
    double b;
  } GTY((desc ("%1.kind"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *data;
  int size;
};

struct GTY((user)) another_user_struct {
  long id;
  char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  double d;
  void *p;
};

union GTY((desc ("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    int start;
    int end;
  } range;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct *my_pointer;
typedef user_struct *user_pointer;
typedef my_union *union_pointer;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array[10];
typedef my_struct *struct_ptr_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*compare_callback)(const void *, const void *);
typedef my_string (*string_generator)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("%0.kind"), tag("LANG_NODE"))) lang_node {
  enum lang_node_kind { LANG_EXPR, LANG_STMT } kind;
  union {
    struct lang_expr *expr;
    struct lang_stmt *stmt;
  } GTY((desc ("%1.kind"))) u;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) container_struct {
  my_array array_field;
  my_pointer ptr_field;
  my_callback callback_field;
  union GTY(()) {
    int as_int;
    double as_double;
  } value;
};

/* Nested pointer/array combinations */
typedef my_struct *(*factory_callback)(int);
typedef my_callback callback_array[5];

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
