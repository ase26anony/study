/* test-base.gtype - Base type definitions for gengtype coverage */

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef int my_scalar_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *filename_t;

/* TYPE_CALLBACK - function pointer */
typedef void (*callback_func)(void *data);

/* TYPE_STRUCT - basic structure */
struct my_struct GTY(())
{
  int id;
  color_enum color;
  filename_t name;
};

/* TYPE_USER_STRUCT */
typedef struct GTY((user)) user_struct
{
  int user_data;
  callback_func callback;
} user_struct_t;

/* TYPE_POINTER - linked list example */
struct list_node GTY(())
{
  int value;
  struct list_node * GTY((skip)) next;  /* TYPE_POINTER */
  struct opaque_struct * GTY((skip)) opaque_ptr;  /* TYPE_POINTER to undefined */
};

/* TYPE_ARRAY - various array types */
struct array_container GTY(())
{
  int fixed_array[10];  /* Fixed-size array */
  int * GTY((length("len"))) dyn_array;  /* Dynamic array */
  size_t len;
  struct my_struct struct_array[5];  /* Array of structs */
};

/* TYPE_UNION */
union data_union GTY(())
{
  int int_val;
  double double_val;
  struct my_struct * GTY((skip)) struct_ptr;
  int array_val[4];
};
