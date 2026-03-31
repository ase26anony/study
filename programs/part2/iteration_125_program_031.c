/* Test types for gengtype coverage - covering all type_kind cases */

#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete/forward declaration */
struct undefined_type_1;
struct undefined_type_2;
struct undefined_type_3;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar_1;
typedef long my_scalar_2;
typedef unsigned char my_scalar_3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string_1;
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h) + 1"))) my_string_3;

/* TYPE_STRUCT: Complete struct definitions */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  int data;
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_val;
    double double_val;
  } value;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_1 {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) user_struct_2 {
  long id;
  const char *name;
};

/* TYPE_UNION: Union types */
union GTY((tag("UNION_1"))) my_union_1 {
  int int_val;
  float float_val;
  double double_val;
};

union GTY((desc("%0.kind"))) my_union_2 {
  int kind;
  struct {
    int x, y;
  } point;
  struct {
    double radius;
  } circle;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct_1 *my_pointer_1;
typedef my_struct_2 * GTY((skip)) my_pointer_2;
typedef const my_struct_3 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5];
typedef GTY((length("sizeof(%h)/sizeof(%h[0])"))) int my_array_3[];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, ...);
typedef void (* GTY((skip)) my_callback_3)(void *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_specific;
  struct lang_struct_type_1 *next;
};

struct GTY((desc("%0.tag"), chain_prev="%h.prev")) lang_struct_type_2 {
  enum lang_tag tag;
  struct lang_struct_type_2 *prev;
  union {
    int ival;
    double dval;
  } data;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with union */
struct GTY(()) complex_nested {
  int type;
  union {
    struct {
      int x, y;
    } point;
    struct {
      double radius;
      int segments;
    } circle;
  } shape;
  my_callback_1 callback;
};

/* Struct with array field */
struct GTY((length("sizeof(%h.data)/sizeof(%h.data[0])"))) struct_with_array {
  int count;
  int data[20];
  my_string_2 name;
};

/* Union with pointer fields */
union GTY((tag("UNION_PTR"))) union_with_pointers {
  my_pointer_1 ptr1;
  my_pointer_2 ptr2;
  my_callback_2 callback;
};

/* Chain of structs for testing traversal */
struct GTY((chain_next("%h.next"))) chain_struct {
  int id;
  my_string_3 name;
  struct chain_struct *next;
  union my_union_1 data;
};

#endif /* TEST_GENGTYPE_TYPES_H */
