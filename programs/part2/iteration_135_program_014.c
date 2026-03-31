/* test-gtypes.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef int my_scalar_type;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct my_base_struct GTY(())
{
  int id;
  const char *name;
  struct my_base_struct *next;
};

/* TYPE_USER_STRUCT - with user-defined options */
typedef struct user_def GTY((user))
{
  int custom_field;
  void *user_data;
} user_struct_t;

/* TYPE_UNION */
union my_union GTY(())
{
  int int_val;
  double double_val;
  struct my_base_struct *struct_ptr;
  const char *string_val;
};

/* TYPE_ARRAY - within a struct */
struct array_container GTY(())
{
  int fixed_array[10];
  struct my_base_struct *ptr_array[5];
  int *variable_array GTY((length("len")));
  size_t len;
};

/* TYPE_POINTER - pointer types */
typedef struct my_base_struct *base_ptr_t;
typedef union my_union *union_ptr_t;

/* Linked list for recursive testing */
struct linked_list GTY(())
{
  int value;
  struct linked_list * GTY((skip)) next;  /* skip to avoid infinite recursion */
  struct linked_list *prev;
};

/* Complex nested structure */
struct complex_nested GTY(())
{
  struct my_base_struct base;
  union my_union data;
  struct array_container arrays;
  callback_func callback;
  string_type description;
};

#endif /* TEST_GTYPES_H */
