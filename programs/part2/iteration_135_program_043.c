/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool boolean_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_STRUCT: Standard struct */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;
  struct base_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct with custom options */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) private_ptr;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
  int int_val;
  double double_val;
  void *ptr_val;
  struct base_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;
typedef callback_func callback_ptr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct base_struct *struct_ptr_array[5];

/* Complex nested structure with multiple type kinds */
struct GTY(()) complex_struct {
  /* TYPE_SCALAR */
  int counter;
  color_enum color;
  
  /* TYPE_STRING */
  const char *filename;
  
  /* TYPE_POINTER */
  struct complex_struct *self_ptr;
  struct opaque_struct *opaque_ptr;
  
  /* TYPE_ARRAY */
  int scores[5];
  struct base_struct *children[3];
  
  /* TYPE_UNION */
  union data_union data;
  
  /* TYPE_CALLBACK */
  callback_func notify;
  
  /* Nested TYPE_STRUCT */
  struct GTY(()) nested {
    int depth;
    struct nested *GTY((skip)) parent;
  } inner;
};

/* Variable length array structure */
struct GTY(()) varray_struct {
  int count;
  int GTY((length ("%h.count"))) items[];
};

#endif /* TEST_GTYPES_H */
