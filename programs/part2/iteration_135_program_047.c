/* test-gtypes.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - basic scalar types */
typedef int scalar_int_t;
typedef enum { RED, GREEN, BLUE } color_enum_t;

/* TYPE_STRING - string type */
typedef const char *string_t;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func_t)(void *data);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_USER_STRUCT - user-defined struct with special handling */
struct user_base {
  int id;
  void *user_data;
};

/* TYPE_STRUCT - regular struct type */
struct linked_node {
  int value;
  struct linked_node *next;  /* TYPE_POINTER */
  struct linked_node *prev;  /* TYPE_POINTER */
};

/* TYPE_ARRAY - array types */
struct array_container {
  int fixed_array[10];           /* fixed-size array */
  int *dynamic_array;            /* pointer for variable-length array */
  struct linked_node *node_array[5]; /* array of pointers */
};

/* TYPE_UNION - union type */
union data_union {
  int int_val;
  double double_val;
  void *ptr_val;
  struct linked_node *node_ptr;
};

/* TYPE_POINTER - standalone pointer type */
typedef struct linked_node *node_ptr_t;
typedef union data_union *union_ptr_t;

/* Complex nested structure combining multiple types */
struct complex_struct {
  /* TYPE_SCALAR */
  int counter;
  enum status { OK, ERROR, PENDING } state;
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_POINTER */
  struct complex_struct *parent;
  struct complex_struct **children;  /* pointer to pointer */
  
  /* TYPE_ARRAY */
  int scores[20];
  struct linked_node *nodes[8];
  
  /* TYPE_UNION */
  union data_union data;
  
  /* TYPE_CALLBACK */
  callback_func_t notify;
  
  /* Reference to undefined type */
  struct opaque_struct *opaque;  /* TYPE_UNDEFINED reference */
};

#endif /* TEST_GTYPES_H */
