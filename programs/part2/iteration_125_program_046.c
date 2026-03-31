/* test-gengtype-types.h - Test types for gengtype coverage testing */
/* This file defines types corresponding to each type_kind enum case */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String types */
typedef const char *my_string_1;
typedef const char *my_string_2 GTY((skip));
typedef const char *my_string_3 GTY((length("strlen(%h) + 1")));

/* TYPE_STRUCT: Complete C structs */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_string_1 field2;
  struct my_struct_1 *next;
};

struct GTY((chain_next("%h.next_ptr"))) my_struct_2 {
  int data;
  struct my_struct_2 *next_ptr;
  my_array_1 arr_field;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_val;
    double dbl_val;
  } value;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) user_struct_1 {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) user_struct_2 {
  long id;
  struct user_struct_2 *next;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  int int_val;
  float float_val;
  double double_val;
};

union GTY((desc("%1.utype"))) my_union_2 {
  int utype;
  struct {
    int x, y;
  } point;
  struct {
    float radius;
    int sides;
  } circle;
};

union my_union_3 {
  my_scalar_1 scalar;
  my_string_1 string;
  struct my_struct_1 *struct_ptr;
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_array_1 *my_pointer_3;

/* TYPE_ARRAY: Fixed-size array types */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef union my_union_2 my_array_3[3] GTY((tag("ARRAY_3")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(struct my_struct_1 *);
typedef void (*my_callback_3)(void) GTY((skip));

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.lang_type"), chain_next="%h.next_lang")) lang_struct_type_1 {
  int lang_type;
  const char *name;
  struct lang_struct_type_1 *next_lang;
  my_callback_1 handler;
};

struct GTY((desc("0"), chain_prev="%p.prev", chain_next="%h.next")) lang_struct_type_2 {
  int id;
  struct lang_struct_type_2 *prev;
  struct lang_struct_type_2 *next;
  union my_union_1 data;
};

/* Additional complex nested types to ensure traversal */
struct GTY(()) complex_nested {
  struct my_struct_1 *struct_field;
  union my_union_2 union_field;
  my_array_1 array_field;
  my_callback_2 callback_field;
  struct lang_struct_type_1 *lang_field;
};

/* Template-like structure with conditional fields */
struct GTY((desc("%1.kind"))) variant_struct {
  enum { KIND_A, KIND_B, KIND_C } kind;
  union {
    struct { int x, y; } a;
    struct { float radius; } b;
    struct { const char *name; int count; } c;
  } u;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
