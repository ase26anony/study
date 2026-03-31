/* test-gtypes.h - Comprehensive test of all gengtype type classifications */
#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR - Basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool scalar_bool;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_type)(int, void*);

/* TYPE_STRUCT - Basic structure */
struct GTY(()) basic_struct {
  int field1;
  scalar_int field2;
  string_type name;
};

/* TYPE_USER_STRUCT - User-defined structure */
typedef struct GTY((user)) user_struct {
  int user_data;
  void * GTY((skip)) opaque_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int int_val;
  double double_val;
  struct basic_struct *struct_ptr;
};

/* TYPE_POINTER - Linked list example */
struct GTY(()) linked_node {
  int data;
  struct linked_node * GTY((skip("next"))) next;
  struct linked_node * GTY((skip("prev"))) prev;
};

/* TYPE_ARRAY - Various array types */
struct GTY(()) array_container {
  int fixed_array[10];
  int * GTY((length("len"))) variable_array;
  size_t len;
  struct basic_struct struct_array[5];
};

/* Recursive structure with multiple type kinds */
struct GTY(()) complex_type {
  struct basic_struct base;          /* TYPE_STRUCT */
  union test_union data;             /* TYPE_UNION */
  struct linked_node *list;          /* TYPE_POINTER */
  callback_type callback;            /* TYPE_CALLBACK */
  string_type description;           /* TYPE_STRING */
  int counters[3];                   /* TYPE_ARRAY of SCALAR */
  color_enum color;                  /* TYPE_SCALAR (enum) */
};

/* Container with all types */
struct GTY(()) type_container {
  struct undefined_struct *undefined_ptr;  /* TYPE_POINTER to UNDEFINED */
  user_struct_t user;                      /* TYPE_USER_STRUCT */
  struct complex_type complex;             /* TYPE_STRUCT */
  union test_union union_field;            /* TYPE_UNION */
  callback_type callbacks[2];              /* TYPE_ARRAY of CALLBACK */
};

#endif /* TEST_GTYPES_H */
