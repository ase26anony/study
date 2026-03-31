/* test-gtypes.h - Comprehensive test of all gengtype type classifications */
#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration without definition */
struct opaque_struct;

/* TYPE_SCALAR - Basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool boolean_type;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_type)(int, void*);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT - Standard struct */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;
  scalar_int value;
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  void *GTY((skip)) user_data;
  int user_id;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int as_int;
  double as_double;
  void *GTY((tag("0"))) as_pointer;
  struct base_struct *GTY((tag("1"))) as_struct;
};

/* TYPE_POINTER - Various pointer types */
struct GTY(()) pointer_container {
  struct base_struct *GTY((skip(""))) next;  /* TYPE_POINTER to TYPE_STRUCT */
  struct opaque_struct *GTY((skip)) opaque_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED */
  user_struct_t *user_ptr;  /* TYPE_POINTER to TYPE_USER_STRUCT */
  callback_type callback_ptr;  /* TYPE_POINTER to TYPE_CALLBACK */
  string_type string_ptr;  /* TYPE_POINTER to TYPE_STRING */
};

/* TYPE_ARRAY - Fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];  /* Fixed-size array of scalars */
  struct base_struct *GTY((length("count"))) var_array;  /* Variable-length array */
  int count;
};

/* Recursive structure for deep processing */
struct GTY(()) recursive_struct {
  int data;
  struct recursive_struct *GTY((skip)) next;  /* Self-referential pointer */
  union test_union value;  /* Contains TYPE_UNION */
};

/* Container with all types */
struct GTY(()) master_container {
  struct base_struct base;          /* TYPE_STRUCT */
  user_struct_t user;               /* TYPE_USER_STRUCT */
  union test_union uni;             /* TYPE_UNION */
  struct pointer_container *ptrs;   /* TYPE_POINTER */
  struct array_container arrays;    /* Contains TYPE_ARRAY */
  scalar_int scalar_field;          /* TYPE_SCALAR */
  string_type string_field;         /* TYPE_STRING */
  callback_type callback_field;     /* TYPE_CALLBACK */
  struct recursive_struct *recursive; /* TYPE_POINTER to recursive struct */
};

#endif /* TEST_GTYPES_H */
