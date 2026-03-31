/* test-gtypes.h - Comprehensive test for gengtype type coverage */

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

/* TYPE_STRUCT - Standard struct */
struct my_struct {
  int id;
  const char *name;  /* TYPE_STRING */
  struct my_struct *next;  /* TYPE_POINTER */
} GTY((tag("MY_STRUCT")));

/* TYPE_USER_STRUCT - User-defined struct */
typedef struct {
  int x;
  int y;
} point GTY((user));

/* TYPE_UNION */
union data_union {
  int int_val;
  float float_val;
  const char *str_val;  /* TYPE_STRING */
  struct my_struct *struct_ptr;  /* TYPE_POINTER */
} GTY((tag("DATA_UNION")));

/* TYPE_ARRAY - Fixed size array */
struct array_container {
  int fixed_array[10];  /* Fixed array */
  int *dynamic_array GTY((length("dynamic_length")));  /* Variable array */
  size_t dynamic_length;
};

/* TYPE_POINTER - Various pointer types */
struct pointer_examples {
  struct my_struct *struct_ptr;      /* Pointer to struct */
  union data_union *union_ptr;       /* Pointer to union */
  callback_type callback_ptr;        /* Pointer to callback */
  struct opaque_struct *opaque_ptr;  /* Pointer to undefined */
  int *int_ptr;                      /* Pointer to scalar */
  const char **string_ptr_ptr;       /* Pointer to string pointer */
};

/* Recursive structure for deep processing */
struct recursive_node {
  int value;
  struct recursive_node *left;   /* TYPE_POINTER */
  struct recursive_node *right;  /* TYPE_POINTER */
  struct recursive_node *parent; /* TYPE_POINTER */
} GTY((tag("RECURSIVE_NODE")));

/* Complex nested structure */
struct complex_nested {
  struct {
    int x;
    int y;
  } nested_struct;
  
  union {
    int i;
    float f;
  } nested_union;
  
  struct recursive_node *tree_root;  /* TYPE_POINTER */
  struct array_container arrays;     /* TYPE_STRUCT containing arrays */
};

#endif /* TEST_GTYPES_H */
