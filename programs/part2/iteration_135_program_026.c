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

/* TYPE_STRUCT - Standard structure */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;  /* TYPE_STRING */
  scalar_int value;              /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT - User-defined structure with special handling */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int as_int;
  double as_double;
  struct base_struct *GTY((tag("0"))) as_struct;
  const char *GTY((tag("1"))) as_string;
};

/* TYPE_ARRAY - Various array types */
struct GTY(()) array_container {
  int fixed_array[10];                     /* Fixed-size array */
  struct base_struct *GTY((length("len"))) dyn_array;  /* Variable-length array */
  size_t len;
  int GTY((atomic)) atomic_array[5];       /* Atomic array */
};

/* TYPE_POINTER - Pointer types forming linked structures */
struct GTY(()) linked_node {
  int data;
  struct linked_node *GTY((skip)) next;    /* TYPE_POINTER */
  struct linked_node *GTY((skip)) prev;    /* TYPE_POINTER */
};

/* Recursive structure with multiple pointer types */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((skip)) left;
  struct tree_node *GTY((skip)) right;
  union test_union node_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_type {
  struct base_struct base;                 /* TYPE_STRUCT */
  union test_union data_union;             /* TYPE_UNION */
  struct array_container arrays;           /* TYPE_STRUCT containing TYPE_ARRAY */
  callback_type callback;                  /* TYPE_CALLBACK */
  string_type description;                 /* TYPE_STRING */
  color_enum color;                        /* TYPE_SCALAR (enum) */
  boolean_type valid;                      /* TYPE_SCALAR (bool) */
  struct opaque_struct *GTY((skip)) opaque_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Container with all type kinds */
struct GTY(()) type_collection {
  /* Scalars */
  int count;
  double ratio;
  boolean_type flag;
  color_enum color;
  
  /* Strings */
  const char *title;
  string_type alias;
  
  /* Structures */
  struct base_struct base;
  user_struct_t user;
  
  /* Unions */
  union test_union data;
  
  /* Arrays */
  int numbers[20];
  struct base_struct *GTY((length("item_count"))) items;
  size_t item_count;
  
  /* Pointers */
  struct complex_type *GTY((skip)) complex;
  struct linked_node *GTY((skip)) list_head;
  
  /* Callbacks */
  callback_type handler;
  compare_fn comparator;
  
  /* Undefined/opaque */
  struct opaque_struct *GTY((skip)) opaque;
};

#endif /* TEST_GTYPES_H */
