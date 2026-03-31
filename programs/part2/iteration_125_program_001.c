/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch branches in gengtype.cc are executed. */

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
typedef const char * GTY((skip)) my_string_2;
typedef const char * GTY((length("strlen(%h)"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((desc("%0"))) my_struct_1 {
  int field1;
  my_scalar_1 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  struct my_struct_2 *next;
  struct my_struct_2 *prev;
  int data;
};

struct GTY((skip)) my_struct_3 {
  void *ptr;
  int count;
};

/* TYPE_USER_STRUCT: User-defined structs with GTY((user)) */
struct GTY((user)) my_user_struct_1 {
  int user_data;
  void *user_ptr;
};

struct GTY((user)) my_user_struct_2 {
  double value;
  struct my_user_struct_2 *next;
};

/* TYPE_UNION: Union types */
union GTY((desc("%0"))) my_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((tag("union_type"))) my_union_2 {
  struct my_struct_1 *s;
  my_scalar_1 i;
  my_string_1 str;
};

union my_union_3 {
  long long big;
  struct {
    int a;
    int b;
  } parts;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_1 * GTY((skip)) my_pointer_2;
typedef my_scalar_1 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef const char * GTY((length("strlen(%h[i])"))) my_array_3[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (*my_callback_2)(const char *, ...);
typedef struct my_struct_1 *(*my_callback_3)(int, void *);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  struct lang_struct_type_1 *next;
  int lang_data;
  enum { LANG_A, LANG_B } lang_tag;
};

struct GTY((desc("%0"), tag("LANG_TYPE"))) lang_struct_type_2 {
  int type_id;
  union {
    int int_val;
    double double_val;
  } u;
};

struct GTY((desc("%d"))) lang_struct_type_3 {
  int discriminator;
  void *data;
};

/* Additional complex types to ensure thorough traversal */
struct GTY(()) complex_container {
  struct my_struct_1 * GTY((skip)) ptr_field;
  my_array_1 array_field;
  my_callback_1 callback_field;
  union my_union_2 union_field;
};

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
  struct GTY((desc("%0"))) inner_struct {
    int inner_data;
    struct inner_struct *next;
  } *inner;
  
  union {
    int x;
    double y;
  } variant;
  
  my_callback_2 callback;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
