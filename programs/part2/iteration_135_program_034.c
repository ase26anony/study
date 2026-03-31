/* test-gtype-all.h - Comprehensive test for gengtype coverage */
#ifndef TEST_GTYPE_ALL_H
#define TEST_GTYPE_ALL_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_STRUCT: Standard C struct with GTY markers */
struct my_struct GTY(())
{
  int a;
  struct my_struct *next;  /* TYPE_POINTER */
  const char *name;        /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: User-defined structure with custom options */
typedef struct user_struct
{
  int id;
  void *data;  /* Opaque pointer */
} user_struct_t;

GTY((user)) user_struct_t;

/* TYPE_UNION: C union type */
union my_union GTY(())
{
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(())
{
  int fixed_array[10];           /* Fixed-size array */
  struct my_struct *var_array GTY((length("len")));  /* Variable-length array */
  int len;
};

/* TYPE_SCALAR: Enum and scalar types */
enum my_enum GTY(())
{
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(int, const char*) GTY((callback));

struct callback_container GTY(())
{
  callback_func handler;
  int priority;
};

/* Recursive structure for deep processing */
struct tree_node GTY(())
{
  int value;
  struct tree_node *left;   /* TYPE_POINTER */
  struct tree_node *right;  /* TYPE_POINTER */
  union my_union data;      /* TYPE_UNION */
};

/* Complex nested structure */
struct complex_struct GTY(())
{
  struct my_struct base;           /* TYPE_STRUCT */
  union my_union variant;          /* TYPE_UNION */
  struct array_container arrays;   /* TYPE_ARRAY container */
  enum my_enum status;             /* TYPE_SCALAR (enum) */
  const char *description;         /* TYPE_STRING */
  struct complex_struct *next;     /* TYPE_POINTER (recursive) */
};

#endif /* TEST_GTYPE_ALL_H */
