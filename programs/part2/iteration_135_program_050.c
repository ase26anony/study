/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool boolean_type;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;  /* TYPE_STRING */
  struct base_struct *next;      /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - struct with user-defined handling */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int int_val;
  float float_val;
  double double_val;
  struct base_struct *struct_ptr;
};

/* TYPE_ARRAY - various array types */
struct GTY(()) array_container {
  int fixed_array[10];                    /* fixed-size array */
  int *GTY((length("len"))) dyn_array;    /* dynamic array */
  size_t len;
  struct base_struct *ptr_array[5];       /* array of pointers */
};

/* TYPE_POINTER - pointer types */
typedef struct base_struct *base_ptr;
typedef union test_union *union_ptr;
typedef int *int_ptr;
typedef const char **string_ptr_ptr;

/* Recursive structure for deep processing */
struct GTY(()) recursive_struct {
  int value;
  struct recursive_struct *GTY((skip("next"))) next;
  struct recursive_struct *prev;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  struct base_struct base;
  union test_union data;
  struct array_container arrays;
  callback_func callback;
  string_type description;
};

#endif /* TEST_GTYPES_H */
