/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_STRUCT - standard C struct */
struct my_struct GTY(())
{
  int a;
  struct my_struct *next;  /* TYPE_POINTER */
  const char *name;        /* TYPE_STRING */
};

/* TYPE_USER_STRUCT - struct with user-defined options */
typedef struct user_struct
{
  long id;
  void *data;  /* opaque pointer */
} user_struct_t;

GTY((user)) user_struct_t;

/* TYPE_UNION */
union my_union GTY(())
{
  int ival;
  float fval;
  struct my_struct *sptr;  /* TYPE_POINTER inside union */
};

/* TYPE_ARRAY - fixed size array */
struct array_container GTY(())
{
  int fixed_array[10];           /* TYPE_ARRAY of TYPE_SCALAR */
  struct my_struct *ptr_array[5]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_SCALAR - basic types */
enum my_enum GTY(()) {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, const char*) GTY((callback));

struct callback_container GTY(())
{
  callback_func handler;
  int callback_id;
};

/* Recursive structure for deep processing */
struct recursive_struct GTY(())
{
  int value;
  struct recursive_struct *left;   /* TYPE_POINTER */
  struct recursive_struct *right;  /* TYPE_POINTER */
  union my_union data;             /* TYPE_UNION */
};

/* Complex nested structure */
struct complex_nested GTY(())
{
  struct my_struct base;
  struct array_container arrays;
  enum my_enum status;
  struct complex_nested *next;
};

/* Variable length array using length option */
struct var_len_struct GTY(())
{
  int count;
  int items[1] GTY((length("%h.count")));
};

/* Chain structure for linked list testing */
struct chain_link GTY(())
{
  int id;
  const char *label;
  struct chain_link *next;
  struct chain_link *prev;
};

/* Container with multiple pointer types */
struct pointer_container GTY(())
{
  void *generic_ptr;           /* void pointer */
  struct my_struct *typed_ptr; /* typed pointer */
  const char *string_ptr;      /* string pointer */
  int *scalar_ptr;             /* scalar pointer */
};
