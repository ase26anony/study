/* test-all-types.gtype - Comprehensive type definitions for gengtype coverage */

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - basic scalar types */
typedef int my_scalar_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, const char *);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct my_struct GTY(()) {
  int id;
  const char *name;
  struct my_struct *next;
};

/* TYPE_USER_STRUCT - struct with user-defined options */
typedef struct user_struct {
  long data;
  void *user_ptr;
} user_struct_t;
GTY((user)) user_struct_t;

/* TYPE_UNION */
union my_union GTY(()) {
  int int_val;
  float float_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_POINTER - standalone pointer type */
typedef struct my_struct *my_struct_ptr_t;

/* TYPE_ARRAY - array types */
struct array_container GTY(()) {
  int fixed_array[10];
  int *variable_array GTY((length("len")));
  size_t len;
};

/* Linked list example with multiple type interactions */
struct complex_node GTY(()) {
  int scalar_field;
  const char *string_field;
  struct complex_node *pointer_field;
  union my_union union_field;
  int array_field[5];
  callback_func callback_field;
  color_enum enum_field;
};

/* Self-referential structure for deep processing */
struct tree_node GTY(()) {
  int value;
  struct tree_node *left;
  struct tree_node *right;
  struct tree_node *parent;
};
