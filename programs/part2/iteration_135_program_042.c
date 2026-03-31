/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - forward declaration */
struct opaque_struct;

/* TYPE_SCALAR - basic types */
typedef int my_scalar_type;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer */
typedef void (*callback_type)(int, void*);

/* TYPE_STRUCT - standard struct */
struct my_struct GTY(()) {
  int field1;
  struct my_struct *next;  /* TYPE_POINTER */
  const char *name;        /* TYPE_STRING */
  callback_type callback;  /* TYPE_CALLBACK */
  color_enum color;        /* TYPE_SCALAR (enum) */
};

/* TYPE_USER_STRUCT */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *user_ptr;
} user_struct_t;

/* TYPE_UNION */
union my_union GTY(()) {
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;
  const char *string_val;
};

/* TYPE_ARRAY - within a struct */
struct array_container GTY(()) {
  int fixed_array[10];           /* Fixed-size array */
  struct my_struct *ptr_array[5]; /* Array of pointers */
  int variable_length GTY((length("len"))); /* Variable-length array */
  int len;
};

/* TYPE_POINTER - standalone typedef */
typedef struct my_struct *my_struct_ptr;

/* Linked list for recursive testing */
struct linked_list GTY(()) {
  int data;
  struct linked_list *next;
  struct linked_list *prev;
};

/* Complex nested structure */
struct complex_nested GTY(()) {
  union my_union u;
  struct array_container arr;
  struct my_struct nested_struct;
  struct complex_nested *self_ptr;
};
