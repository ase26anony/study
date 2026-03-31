/* Test types for gengtype coverage - covering all type_kind cases */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declarations */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char *my_string_2 GTY(());
typedef const char *my_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((skip)) my_struct_2 {
  int a;
  double b;
  const char *c;
};

struct GTY((chain_next = "%h.next")) my_struct_3 {
  my_struct_1 *data;
  my_struct_3 *next;
  int value;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct_1 {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) my_user_struct_2 {
  long custom_field;
  void *private_ptr;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%0.as_int"))) my_union_2 {
  int type;
  struct my_struct_1 *s;
  union my_union_1 *u;
};

union my_union_3 {
  long long_val;
  const char *str_val;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct_1 *my_pointer_1;
typedef my_struct_2 * GTY(()) my_pointer_2;
typedef const my_union_1 * const my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5] GTY(());
typedef const char *my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(my_struct_1 *, my_scalar_1) GTY(());
typedef void (*my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type_1 {
  int lang_specific_tag;
  struct lang_struct_type_1 *next;
  void *lang_data;
};

struct GTY((desc("%0.type"), tag("LANG_STRUCT_2"))) lang_struct_type_2 {
  enum { LANG_TYPE_A, LANG_TYPE_B } type;
  union {
    my_struct_1 *as_struct;
    my_union_1 *as_union;
  } u;
};

struct GTY((desc("1"), skip)) lang_struct_type_3 {
  int dummy_field;
  struct lang_struct_type_1 *chain;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with multiple pointer types */
struct GTY(()) complex_nested {
  my_struct_1 base;
  my_pointer_2 ptr_field;
  my_array_1 array_field;
  my_callback_1 callback_field;
  struct GTY((tag("COMPLEX_SUB"))) {
    int sub_field1;
    double sub_field2;
  } sub;
};

/* Union containing various types */
union GTY((desc("%0.tag"))) variant_type {
  struct {
    int tag;
    union {
      my_struct_1 *s;
      my_union_2 *u;
      my_array_2 a;
    } data;
  } tagged;
  long raw_data[4];
};

/* Struct with array of pointers */
struct GTY(()) struct_with_ptr_array {
  int count;
  my_struct_1 * GTY((length("%h.count"))) items[1];
};

/* Callback in struct */
struct GTY(()) struct_with_callback {
  const char *name;
  my_callback_2 handler;
  void *user_data;
};

#endif /* TEST_GENGTYPE_TYPES_H */
