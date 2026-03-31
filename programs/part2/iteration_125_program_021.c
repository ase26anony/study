/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure gengtype processes each category. */

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
typedef const char * GTY((length("strlen(%0)+1"))) my_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_scalar_3 field1;
  my_string_2 field2;
  struct my_struct_1 * GTY((skip)) field3;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  enum { TYPE_A, TYPE_B, TYPE_C } type;
  union {
    my_scalar_1 a;
    my_string_1 b;
    struct my_struct_1 *c;
  } GTY((desc("%0.type"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) user_struct_1 {
  int user_field1;
  void *user_field2;
};

struct GTY((user)) user_struct_2 {
  double user_data;
  struct user_struct_1 *related;
};

struct GTY((user)) user_struct_3 {
  char name[32];
  int id;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  my_scalar_1 as_scalar;
  my_string_1 as_string;
  struct my_struct_1 *as_struct;
};

union GTY((desc("%1.tag"))) my_union_2 {
  int tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    const char *name;
    int value;
  } named;
};

union my_union_3 {
  long long_data;
  double double_data;
  void *ptr_data;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_string_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length("strlen(%0[i])+1"))) my_array_3[8];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(struct my_struct_1 *, union my_union_1);
typedef struct my_struct_2 * (*my_callback_3)(my_string_1, my_scalar_2);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.lang_type"), chain_next="%h.next")) lang_struct_1 {
  int lang_type;
  const char *name;
  struct lang_struct_1 *next;
};

struct GTY((desc("TREE_CODE(%0)"))) lang_struct_2 {
  enum tree_code code;
  union lang_tree_node * GTY((tag("TREE_CODE(%0.code)"))) u;
  struct lang_struct_2 * GTY((skip)) parent;
};

struct GTY((desc("%1.kind"), chain_next="%h.next")) lang_struct_3 {
  enum { KIND_A, KIND_B, KIND_C } kind;
  int data;
  struct lang_struct_3 *next;
};

/* Additional struct to ensure multiple instances are counted */
struct GTY(()) extra_struct_1 {
  my_array_1 arr;
  my_callback_1 cb;
};

struct GTY((tag("EXTRA_2"))) extra_struct_2 {
  union my_union_2 u;
  my_pointer_2 ptr;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
