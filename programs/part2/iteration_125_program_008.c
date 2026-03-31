/* Test types for gengtype coverage - covering all type_kind enum cases */
/* This file should be included in gtype-desc.cc */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2;
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) my_struct_2 {
  int data;
  struct my_struct_2 *GTY((skip)) next;
  struct my_struct_2 *prev;
};

struct GTY((desc ("%0.kind"))) my_struct_3 {
  enum { KIND_A, KIND_B, KIND_C } kind;
  union {
    int int_val;
    double double_val;
    my_string_1 str_val;
  } GTY((desc ("%1.kind"))) u;
};

/* TYPE_USER_STRUCT: Structs with user tag */
struct GTY((user)) my_user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) my_user_struct_2 {
  double user_value;
  const char *user_name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int int_member;
  double double_member;
  void *ptr_member;
};

union GTY((desc ("%0.tag"))) my_union_2 {
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

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct_1 *my_pointer_1;
typedef my_struct_2 *GTY((skip)) my_pointer_2;
typedef const my_struct_3 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5];
typedef const char *my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char *);
typedef int (*my_callback_2)(my_struct_1 *, my_struct_2 *);
typedef void (*my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc ("%1.type"), tag ("LANG_TYPE"))) lang_struct_type_1 {
  enum lang_type { LANG_INT, LANG_FLOAT, LANG_STRING } type;
  union {
    int int_val;
    double float_val;
    const char *str_val;
  } GTY((desc ("%1.type"))) value;
  struct lang_struct_type_1 *GTY((chain_next ("%h.next"))) next;
};

struct GTY((desc ("%0.kind"), chain_next ("%h.next"))) lang_struct_type_2 {
  enum { EXPR_KIND, STMT_KIND, DECL_KIND } kind;
  int lineno;
  struct lang_struct_type_2 *next;
};

/* Additional struct to ensure multiple instances */
struct GTY(()) extra_struct_1 {
  my_scalar_1 a;
  my_scalar_2 b;
  my_string_1 c;
  my_array_1 d;
};

struct GTY((chain_next ("%h.next"))) extra_struct_2 {
  int id;
  my_union_1 data;
  struct extra_struct_2 *next;
};

/* More pointer types for coverage */
typedef my_union_1 *union_ptr_t;
typedef my_callback_1 callback_ptr_t;
typedef lang_struct_type_1 *lang_ptr_t;

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
