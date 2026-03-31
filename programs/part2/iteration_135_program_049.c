/* Test file to cover all gengtype type cases */
#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - forward declaration */
struct opaque_struct;

/* TYPE_SCALAR - basic types */
typedef int my_scalar_type;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *filename_type;

/* TYPE_CALLBACK - function pointer */
typedef void (*callback_func)(int, void*);

/* TYPE_STRUCT - regular struct */
struct my_struct GTY(()) {
  int field1;
  double field2;
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - user-defined struct */
typedef struct {
  int user_data;
  void *user_ptr;
} user_struct_type;
GTY((user)) user_struct_type;

/* TYPE_UNION */
union my_union GTY(()) {
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY - various array types */
struct array_container GTY(()) {
  int fixed_array[10];           /* fixed-size array */
  struct my_struct *var_array;   /* variable-length array pointer */
  int array_length;
};

/* Linked list example for recursion */
struct linked_list GTY(()) {
  int data;
  struct linked_list *next;
  struct linked_list *prev;
};

/* Complex nested structure */
struct complex_struct GTY(()) {
  union my_union u;
  struct array_container arr;
  callback_func callback;
  filename_type name;
  color_enum color;
};
