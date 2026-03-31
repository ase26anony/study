/* test-gtypes.h - Comprehensive test of all GTY type kinds */
#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - Basic scalar types */
typedef int my_scalar_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_func)(void *data);

/* TYPE_STRUCT - Standard struct */
struct my_struct GTY(())
{
  int id;
  struct my_struct *next;  /* TYPE_POINTER */
  const char *name;        /* TYPE_STRING */
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct user_struct
{
  int user_data;
  void *user_ptr;
} user_struct_t;
#define GTY_USER_STRUCT user_struct_t

/* TYPE_UNION */
union my_union GTY(())
{
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY - Fixed-size array */
struct array_container GTY(())
{
  int fixed_array[10];           /* Fixed array */
  struct my_struct *var_array GTY((length("len")));  /* Variable array */
  int len;
};

/* TYPE_POINTER - Various pointer types */
typedef struct my_struct *struct_ptr_t;
typedef union my_union *union_ptr_t;

/* Recursive structure for deep processing */
struct tree_node GTY(())
{
  int value;
  struct tree_node *left;   /* TYPE_POINTER */
  struct tree_node *right;  /* TYPE_POINTER */
  struct tree_node *parent; /* TYPE_POINTER */
};

/* Complex nested structure */
struct container GTY(())
{
  struct my_struct embedded;      /* TYPE_STRUCT */
  union my_union data_union;      /* TYPE_UNION */
  struct array_container arrays;  /* TYPE_STRUCT containing TYPE_ARRAY */
  callback_func callback;         /* TYPE_CALLBACK */
};

#endif /* TEST_GTYPES_H */
