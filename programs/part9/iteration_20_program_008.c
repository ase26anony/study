/* Base type definitions covering STRUCT, UNION, SCALAR, POINTER, ARRAY */

%{ 
#include "config.h"
#include "system.h"
#include "ansidecl.h"
%}

/* TYPE_SCALAR definitions */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  int scalar_field;
  unsigned long ulong_field;
  color_enum enum_field;
};

/* TYPE_UNION definition */
union my_variant GTY((desc("type_tag")))
{
  int int_val;
  double double_val;
  struct my_base_struct *GTY((tag("1"))) struct_ptr;
  const char *GTY((tag("2"))) string_val;
};

/* TYPE_POINTER definitions */
typedef struct my_base_struct *my_struct_ptr;
typedef union my_variant *my_variant_ptr;

/* TYPE_ARRAY definitions */
struct array_container GTY(())
{
  int count;
  struct my_base_struct *GTY((length("%h.count"))) items[];
};

struct fixed_array GTY(())
{
  struct my_base_struct elements[10];
  int data[20];
};

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_UNDEFINED - forward declaration */
struct forward_declared;
struct uses_forward GTY(())
{
  struct forward_declared *GTY((maybe_undef)) forward_ptr;
};

/* Now define the forward declared struct */
struct forward_declared GTY(())
{
  int value;
};
