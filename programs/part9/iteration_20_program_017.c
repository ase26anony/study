/* test-base.gtype - Base type definitions for gengtype coverage */

/* TYPE_SCALAR definitions */
typedef int my_scalar_int;
typedef unsigned long my_scalar_ulong;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING definition */
typedef const char *my_string_type;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_int id;
  my_string_type name;
  struct undefined_struct *GTY((maybe_undef)) undef_ptr;  /* May trigger TYPE_UNDEFINED */
  color_enum color;
};

/* TYPE_UNION */
union my_base_union GTY(())
{
  my_scalar_int as_int;
  my_string_type as_string;
  struct my_base_struct *GTY((skip)) as_struct;  /* Skip GC for this field */
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int count;
  struct my_base_struct *GTY((length("count"))) items[];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  union my_base_union GTY((tag("color"))) fixed_items[10];
};

/* TYPE_POINTER - various pointer types */
typedef struct my_base_struct *struct_ptr;
typedef union my_base_union *union_ptr;
typedef my_scalar_int *scalar_ptr;

/* Nested structure with pointer to union */
struct complex_struct GTY(())
{
  struct_ptr first;
  union_ptr second;
  struct array_container *GTY((chain_next("next"))) next;
  struct complex_struct *GTY((chain_prev("prev"))) prev;
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(())
{
  int value;
  struct complex_struct *link;
};
