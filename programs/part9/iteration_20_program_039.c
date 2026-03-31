/* Test base types for gengtype coverage */
%{ #include "config.h" %}
%{ #include "system.h" %}
%{ #include "ansidecl.h" %}

/* TYPE_SCALAR */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  int scalar_field;
  unsigned long ulong_field;
  const char *string_field GTY((skip));
  struct undefined_struct *undefined_ptr;
  struct my_base_struct *next GTY((chain_next));
};

/* TYPE_UNION */
union my_union GTY(())
{
  int int_val;
  struct my_base_struct *struct_ptr;
  const char *string_val;
};

/* TYPE_POINTER */
typedef struct my_base_struct *base_struct_ptr;
typedef union my_union *union_ptr_t;

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int length;
  struct my_base_struct *elements GTY((length("%h.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  int data[10];
  struct my_base_struct *ptr_array[5];
};

/* Linked list structure for chain_next/chain_prev */
struct linked_list GTY(())
{
  int value;
  struct linked_list *next GTY((chain_next));
  struct linked_list *prev GTY((chain_prev));
};
