/* test-main.gtype - Main test file covering most type kinds */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_undefined;

/* TYPE_STRUCT: Standard struct with GTY marker */
struct GTY(()) my_struct {
  int a;
  double b;
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int id;
  const char *name;  /* TYPE_STRING */
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY: Fixed-size array within a struct */
struct GTY(()) array_container {
  int fixed_array[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  struct my_struct *ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_SCALAR: Various scalar types */
enum GTY(()) my_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_STRING: String pointer type */
typedef const char * GTY((string)) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) callback_func)(int, const char*);

/* Recursive structure to ensure deep processing */
struct GTY(()) recursive_struct {
  int value;
  struct recursive_struct * GTY((skip)) left;  /* TYPE_POINTER */
  struct recursive_struct * GTY((skip)) right; /* TYPE_POINTER */
};

/* Variable length array using length specifier */
struct GTY(()) var_array_struct {
  int count;
  int items GTY((length("%0.count"))) [];  /* TYPE_ARRAY with variable length */
};

/* Nested structure with union */
struct GTY(()) complex_nested {
  union GTY(()) {
    int i;
    double d;
  } data;
  struct array_container container;
};
