/* test_structures.h - Header with diverse type definitions for gengtype coverage */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  struct undefined_type *undef_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_struct {
  void *opaque_data;
  int user_tag;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
  int int_val;
  float float_val;
  void *ptr_val;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef struct my_struct *struct_ptr;
typedef union my_union *union_ptr;
typedef int *int_ptr;

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func)(int, void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
  int lang_field1;
  void *lang_field2;
};
#endif

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
  struct my_struct inner;
  union my_union data;
  callback_func callback;
  int_array numbers;
};

/* Another structure with pointer chain */
struct GTY(()) pointer_chain {
  struct pointer_chain * GTY((skip)) next;
  struct my_struct *data;
  my_string_type name;
};

#endif /* TEST_STRUCTURES_H */
