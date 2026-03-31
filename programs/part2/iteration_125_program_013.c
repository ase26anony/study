/* Test types for gengtype coverage testing.
   This file defines types corresponding to all type_kind enum cases
   to ensure gengtype processes each category. */

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
typedef const char * GTY((length("strlen(%h)+1"))) my_string_3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((tag("STRUCT_1"))) my_struct_1 {
  my_scalar_1 field1;
  my_scalar_2 field2;
  my_string_1 field3;
};

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) my_struct_2 {
  my_struct_1 *next;
  my_struct_1 *prev;
  my_scalar_3 data;
};

struct GTY((desc("%1.type"))) my_struct_3 {
  int type;
  union {
    my_scalar_1 scalar_val;
    my_string_1 string_val;
  } GTY((desc("%1.type"))) u;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) my_user_struct_1 {
  void *user_data;
  int user_id;
};

struct GTY((user)) my_user_struct_2 {
  my_scalar_1 *data_ptr;
  size_t data_size;
};

/* TYPE_UNION: Union types */
union GTY((desc("%0.type"))) my_union_1 {
  my_scalar_1 scalar;
  my_string_1 string;
  struct my_struct_1 *struct_ptr;
};

union GTY((tag("UNION_2"))) my_union_2 {
  int int_val;
  double double_val;
  void *ptr_val;
};

union my_union_3 {
  my_scalar_1 s1;
  my_scalar_2 s2;
  my_scalar_3 s3;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct_1 *my_pointer_1;
typedef my_struct_2 * GTY((skip)) my_pointer_2;
typedef const my_struct_3 *my_pointer_3;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array_1[10];
typedef my_struct_1 *my_array_2[5];
typedef const char * GTY((length("strlen(%h)+1"))) my_array_3[20];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback_1)(int, const char*);
typedef int (*my_callback_2)(my_struct_1 *, my_scalar_1);
typedef void (* GTY((skip)) my_callback_3)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1.type"), chain_next="%h.next")) lang_struct_type_1 {
  int type;
  lang_struct_type_1 *next;
  my_scalar_1 data;
};

struct GTY((desc("TREE_CODE(%h)"), chain_next="%h.next")) lang_struct_type_2 {
  int code;
  lang_struct_type_2 *next;
  my_string_1 name;
};

struct GTY((desc("%1.tag"), chain_next="%h.next", chain_prev="%h.prev")) lang_struct_type_3 {
  int tag;
  lang_struct_type_3 *next;
  lang_struct_type_3 *prev;
  union my_union_1 data;
};

/* Additional structs to ensure multiple instances are counted */
struct GTY(()) extra_struct_1 {
  my_array_1 arr;
  my_callback_1 cb;
};

struct GTY((tag("EXTRA_2"))) extra_struct_2 {
  my_pointer_1 ptr;
  my_union_2 uni;
};

/* Test structure containing all type kinds */
struct GTY((desc("%1.kind"))) test_container {
  int kind;
  
  /* TYPE_UNDEFINED */
  struct undefined_type_1 *undef_ptr;
  
  /* TYPE_SCALAR */
  my_scalar_1 scalar_field;
  
  /* TYPE_STRING */
  my_string_1 string_field;
  
  /* TYPE_STRUCT */
  struct my_struct_1 struct_field;
  
  /* TYPE_USER_STRUCT */
  struct my_user_struct_1 *user_struct_field;
  
  /* TYPE_UNION */
  union my_union_1 union_field;
  
  /* TYPE_POINTER */
  my_pointer_1 pointer_field;
  
  /* TYPE_ARRAY */
  my_array_1 array_field;
  
  /* TYPE_CALLBACK */
  my_callback_1 callback_field;
  
  /* TYPE_LANG_STRUCT */
  struct lang_struct_type_1 *lang_struct_field;
};

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
