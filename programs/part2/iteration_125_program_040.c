/* Test types for gengtype coverage testing.
   This file defines types corresponding to each type_kind enum case
   to ensure all switch cases in gengtype.cc are executed. */

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
typedef const char * GTY(()) my_string_2;
typedef const char * GTY((skip)) my_string_3;

/* TYPE_STRUCT: Complete C structs with GTY annotations */
struct GTY(()) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((desc("%1"), tag("TAG1"))) my_struct_2 {
  int id;
  struct my_struct_1 * GTY((skip)) next;
  my_scalar_3 value;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_3 {
  struct my_struct_3 *next;
  struct my_struct_3 *prev;
  my_string_2 name;
  my_array_1 data;
};

/* TYPE_USER_STRUCT: Structs with user annotation */
struct GTY((user)) my_user_struct_1 {
  int user_data;
  void *opaque_ptr;
};

struct GTY((user)) my_user_struct_2 {
  long handle;
  const char *description;
};

struct GTY((user)) my_user_struct_3 {
  double x, y, z;
  struct my_user_struct_1 *related;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union_1 {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%0"))) my_union_2 {
  my_scalar_1 scalar;
  my_string_1 string;
  struct my_struct_1 *struct_ptr;
};

union GTY((tag("UNION3"))) my_union_3 {
  struct my_struct_2 s;
  union my_union_1 u;
  my_array_2 arr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct_1 *my_pointer_1;
typedef union my_union_2 * GTY(()) my_pointer_2;
typedef my_callback_1 * GTY((skip)) my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef struct my_struct_1 my_array_2[5];
typedef union my_union_1 GTY(()) my_array_3[3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int);
typedef int (* GTY(()) my_callback_2)(const char *, double);
typedef struct my_struct_1 *(*my_callback_3)(my_scalar_1, my_string_1);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type_1 {
  int lang_id;
  struct lang_struct_type_1 *next;
  my_string_2 lang_name;
};

struct GTY((desc("%0"), tag("LANG2"))) lang_struct_type_2 {
  enum { LANG_A, LANG_B, LANG_C } lang_kind;
  union my_union_3 data;
  my_callback_2 handler;
};

struct GTY((desc("%1"), chain_next="%h.link", chain_prev="%h.prev_link")) 
  lang_struct_type_3 {
  int type_id;
  struct lang_struct_type_3 *link;
  struct lang_struct_type_3 *prev_link;
  my_pointer_2 ptr;
  my_array_3 matrix;
};

/* Additional struct to reference all types */
struct GTY(()) master_test_struct {
  /* Scalars */
  my_scalar_1 s1;
  my_scalar_2 s2;
  my_scalar_3 s3;
  
  /* Strings */
  my_string_1 str1;
  my_string_2 str2;
  my_string_3 str3;
  
  /* Structs */
  struct my_struct_1 *struct1;
  struct my_struct_2 struct2;
  struct my_struct_3 *struct3;
  
  /* User structs */
  struct my_user_struct_1 user1;
  struct my_user_struct_2 *user2;
  struct my_user_struct_3 user3;
  
  /* Unions */
  union my_union_1 union1;
  union my_union_2 *union2;
  union my_union_3 union3;
  
  /* Pointers */
  my_pointer_1 ptr1;
  my_pointer_2 ptr2;
  my_pointer_3 ptr3;
  
  /* Arrays */
  my_array_1 arr1;
  my_array_2 arr2;
  my_array_3 arr3;
  
  /* Callbacks */
  my_callback_1 cb1;
  my_callback_2 cb2;
  my_callback_3 cb3;
  
  /* Lang structs */
  struct lang_struct_type_1 *lang1;
  struct lang_struct_type_2 lang2;
  struct lang_struct_type_3 *lang3;
  
  /* Self-reference for testing */
  struct master_test_struct *next;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
