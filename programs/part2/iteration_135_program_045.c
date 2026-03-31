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
typedef void (*callback_type)(int, void*);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;  /* TYPE_STRING */
  scalar_int value;              /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT - struct with user-defined handling */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int as_int;
  float as_float;
  struct base_struct *GTY((tag("0"))) as_struct;
};

/* TYPE_POINTER - various pointer types */
struct GTY(()) pointer_container {
  struct base_struct *GTY((skip)) next;      /* pointer to struct */
  struct opaque_struct *GTY((skip)) opaque;  /* pointer to undefined */
  user_struct_t *GTY((skip)) user;           /* pointer to user struct */
  callback_type GTY((skip)) callback;        /* pointer to callback */
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];                     /* fixed array of scalars */
  struct base_struct *GTY((length("len"))) var_array; /* variable array */
  int len;
};

/* Recursive structure for deep processing */
struct GTY(()) recursive_node {
  int data;
  struct recursive_node *GTY((skip)) next;
  struct recursive_node *GTY((skip)) prev;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  union test_union data_union;
  struct array_container arrays;
  struct pointer_container pointers;
  callback_type handlers[5];
};

#endif /* TEST_GTYPES_H */
