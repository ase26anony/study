/* Base type definitions covering STRUCT, UNION, SCALAR, POINTER, ARRAY */

%{ 
#include "config.h"
#include "system.h"
#include "ansidecl.h"
%}

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  int scalar_field;
  unsigned long ulong_field;
  color_enum enum_field;
  const char * GTY((skip)) ignored_string;
  struct undefined_struct *forward_ptr;
};

/* TYPE_UNION */
union my_union GTY(())
{
  int int_val;
  double double_val;
  struct my_base_struct *struct_ptr;
  const char *string_val;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int length;
  struct my_base_struct * GTY((length ("%h.length"))) items[];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  int data[10];
  union my_union unions[5];
};

/* TYPE_POINTER - various pointer types */
typedef struct my_base_struct *base_ptr_t;
typedef union my_union *union_ptr_t;
typedef int *int_ptr_t;

/* Complex nested structure */
struct complex_struct GTY(())
{
  struct my_base_struct base;
  union my_union variant;
  struct array_container *dynamic_array;
  base_ptr_t pointer_field;
  int_ptr_t int_pointer;
  
  /* Chain for linked list */
  struct complex_struct * GTY((chain_next ("%h.next"))) next;
  struct complex_struct * GTY((chain_prev ("%h.prev"))) prev;
};

/* TYPE_UNDEFINED - now define it */
struct undefined_struct GTY(())
{
  int defined_now;
  struct complex_struct *complex_ptr;
};
