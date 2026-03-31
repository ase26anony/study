/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch cases in gengtype.cc are executed. */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declared types */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef unsigned long my_scalar_2;
typedef double my_scalar_3;

/* TYPE_STRING: String pointer types */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_struct_1 * GTY((skip)) ptr_field;
  my_scalar_3 array_field[5];
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  enum { TYPE_A, TYPE_B, TYPE_C } type;
  union {
    my_scalar_1 a;
    my_scalar_2 b;
    my_string_1 c;
  } GTY((desc("%0.type"))) value;
};

/* TYPE_USER_STRUCT: User-managed structs */
struct GTY((user)) my_user_struct_1 {
  void *user_data;
  int user_flag;
};

struct GTY((user)) my_user_struct_2 {
  my_user_struct_1 *next;
  char *buffer;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  my_scalar_1 as_int;
  my_scalar_3 as_double;
  my_string_1 as_string;
};

union GTY((desc("%1.tag"))) my_union_2 {
  struct {
    int tag;
    union my_union_2 *next;
  } header;
  my_scalar_2 value;
  my_struct_1 *struct_ptr;
};

union my_union_3 {
  long long_data;
  double double_data;
  void *ptr_data;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct_1 *my_pointer_1;
typedef my_struct_2 * GTY((skip)) my_pointer_2;
typedef union my_union_1 *my_pointer_3;
typedef my_user_struct_1 * GTY((user)) my_pointer_4;

/* TYPE_ARRAY: Array types */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5];
typedef const char * GTY((length("strlen(%h[i])"))) my_array_3[3];
typedef union my_union_2 my_array_4[8];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(my_struct_1 *, my_scalar_2);
typedef my_struct_2 * (*my_callback_3)(my_array_1, my_callback_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.lang_type"), chain_next="%h.next")) lang_struct_type_1 {
  enum lang_type { LANG_A, LANG_B, LANG_C } lang_type;
  my_scalar_1 lang_data;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%1.tag"), chain_prev="%h.prev")) lang_struct_type_2 {
  int tag;
  union {
    my_scalar_2 num;
    my_string_1 str;
    struct lang_struct_type_2 *child;
  } GTY((desc("%0.tag"))) value;
  struct lang_struct_type_2 *prev;
};

struct GTY((desc("%1.kind"))) lang_struct_type_3 {
  enum { KIND_X, KIND_Y, KIND_Z } kind;
  my_callback_1 handler;
  my_array_2 items;
};

/* Additional types to ensure multiple instances */
struct GTY((tag("EXTRA_STRUCT"))) extra_struct_1 {
  my_pointer_1 ptr;
  my_array_1 arr;
  my_callback_1 cb;
};

union extra_union_1 {
  struct extra_struct_1 *s;
  lang_struct_type_1 *l;
  my_user_struct_2 *u;
};

typedef extra_union_1 *extra_pointer_1;
typedef lang_struct_type_2 extra_array_1[4];

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
