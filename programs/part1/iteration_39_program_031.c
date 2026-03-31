/* Test header for gengtype coverage - covers all type categories in statistics */
#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_base_struct {
  int field1;
  my_scalar_t field2;
};

/* TYPE_USER_STRUCT: User-defined struct (forward declared then defined) */
struct my_user_struct;
typedef struct my_user_struct *my_user_ptr;

struct GTY((user)) my_user_struct {
  int data;
  struct my_base_struct * GTY((skip)) link;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  const char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_base_struct * GTY(()) global_struct_ptr;
extern my_scalar_t * GTY((length("array_length"))) dynamic_array_ptr;

/* TYPE_ARRAY: Fixed-size array */
extern int GTY(()) fixed_array[10];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY((desc("%0.type"), tag("MY_LANG_TYPE"))) my_lang_struct {
  enum my_lang_type type;
  union {
    int int_val;
    double float_val;
  } GTY((desc("%1.type"))) u;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;                    /* TYPE_SCALAR */
  const char * GTY((skip)) string_field;       /* TYPE_STRING */
  struct my_base_struct struct_field;          /* TYPE_STRUCT */
  union my_test_union union_field;             /* TYPE_UNION */
  struct my_user_struct * GTY((skip)) ptr_field; /* TYPE_POINTER */
  callback_func callback_field;                /* TYPE_CALLBACK */
  int array_field[5];                          /* TYPE_ARRAY */
};

/* Template-like structure for arrays of pointers */
struct GTY(()) ptr_array_container {
  int count;
  struct my_base_struct * GTY((length("%h.count"))) items[1];
};

/* Another union with GTY markers on members */
union GTY(()) tagged_union {
  int GTY((tag("0"))) as_int;
  struct my_base_struct * GTY((tag("1"))) as_ptr;
  callback_func GTY((tag("2"))) as_func;
};

#endif /* GCC_MYTEST_H */
