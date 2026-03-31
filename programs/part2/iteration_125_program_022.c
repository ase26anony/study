/* Test types for gengtype coverage testing.
   This file defines types for each enum type_kind case in gengtype.cc */

#ifndef GCC_TEST_GENGTYPE_TYPES_H
#define GCC_TEST_GENGTYPE_TYPES_H

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef char *my_mutable_string;
typedef const char * const my_const_string_ptr;

/* TYPE_STRUCT: Complete C structs */
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

struct GTY((desc ("%1.type"))) tagged_struct {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_value;
    my_string string_value;
  } data;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *opaque_data;
  int user_defined_field;
};

struct GTY((user)) another_user_struct {
  long custom_data;
  void (*custom_cleanup)(void*);
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int int_val;
  double double_val;
  my_string string_val;
};

union GTY((desc ("%0.kind"))) tagged_union {
  enum { KIND_INT, KIND_STRING } kind;
  struct {
    int int_value;
  } int_data;
  struct {
    my_string string_value;
  } string_data;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef my_union *union_pointer;
typedef user_struct *user_struct_pointer;
typedef int *int_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, my_string);
typedef int (*comparison_callback)(const void *, const void *);
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int lang_specific_field;
  struct lang_struct_type *next;
};

struct GTY((desc("%1"), tag("LANG_TYPE_A"))) another_lang_struct {
  enum lang_type { LANG_TYPE_A, LANG_TYPE_B } type;
  union {
    int int_field;
    double double_field;
  } data;
};

/* Additional types to ensure multiple instances */

/* More structs for TYPE_STRUCT */
struct GTY(()) extra_struct_one {
  my_array array_field;
  my_callback callback_field;
};

struct GTY((skip)) extra_struct_two {
  union_pointer union_ptr;
  int_pointer int_ptr;
};

/* More unions for TYPE_UNION */
union GTY(()) extra_union {
  my_pointer struct_ptr;
  user_struct_pointer user_ptr;
};

/* More arrays for TYPE_ARRAY */
typedef union_pointer pointer_array[8];
typedef my_callback callback_array[4];

/* Complex nested type to exercise multiple paths */
struct GTY(()) complex_nested {
  struct GTY((desc("%1.inner_type"))) inner_struct {
    enum { INNER_A, INNER_B } inner_type;
    union {
      int nested_int;
      my_string nested_string;
    } inner_data;
  } inner;
  
  pointer_array ptrs;
  callback_array callbacks;
  tagged_union variant;
};

/* Test variable declarations to ensure types are used */
extern my_scalar test_scalar_var;
extern my_string test_string_var;
extern struct my_struct test_struct_var;
extern struct user_struct test_user_struct_var;
extern union my_union test_union_var;
extern my_pointer test_pointer_var;
extern my_array test_array_var;
extern my_callback test_callback_var;
extern struct lang_struct_type test_lang_struct_var;

#endif /* GCC_TEST_GENGTYPE_TYPES_H */
