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
  char field2;
  my_scalar field3;
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_struct {
  void *data;
  int size;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
  int int_val;
  float float_val;
  char * GTY((string)) str_val;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_func)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
  int lang_field1;
  void *lang_field2;
};
#endif

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_field;
  } inner;
  
  union GTY(()) inner_union {
    int a;
    float b;
  } u;
  
  my_struct_ptr ptr_field;
  int_array array_field;
  comparison_func callback_field;
};

/* Another structure with various pointer types */
struct GTY(()) pointer_struct {
  int *int_ptr;
  char **string_ptr_ptr;
  struct my_struct *struct_ptr;
  union my_union *union_ptr;
  void (*func_ptr)(void);
  int (*array_ptr)[10];
};

#endif /* TEST_STRUCTURES_H */
