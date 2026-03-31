/* Header file with various GTY-annotated types to exercise gengtype statistics collection.
   This should be placed in the gcc/ directory of the GCC source tree. */

#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for types we'll use */
struct my_base_struct;
typedef struct my_base_struct *my_handle;

/* TYPE_SCALAR: Simple scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_version_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for variety */
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct GTY(()) my_user_struct {
  int data;
  struct my_struct * GTY((tag("0"))) link;
} *my_user_struct_p;

/* TYPE_UNION: Union type with GTY annotation */
union GTY(()) my_union {
  int as_int;
  double as_double;
  void * GTY((skip)) as_pointer;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_struct * GTY(()) global_struct_ptr;
extern my_user_struct_p GTY(()) user_struct_ptr;
extern int * GTY((length("10"))) int_array_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) fixed_array[10];
extern struct my_struct GTY(()) struct_array[5];
extern const char * GTY((length("my_array_length"))) string_array[];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct {
  int lang_specific_data;
  struct my_struct * GTY((chain_next("%0.next"))) next;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) container {
  /* Contains one of each type category */
  my_scalar_t scalar_field;           /* TYPE_SCALAR */
  const char * GTY(()) string_field;  /* TYPE_STRING */
  struct my_struct struct_field;      /* TYPE_STRUCT */
  union my_union union_field;         /* TYPE_UNION */
  struct container * GTY(()) self_ptr; /* TYPE_POINTER */
  callback_func callback_field;       /* TYPE_CALLBACK */
  
  /* Variable length array */
  int GTY((length("%0.dyn_length"))) *dyn_array; /* TYPE_ARRAY with length */
  int dyn_length;
};

/* Another user struct type for more coverage */
typedef struct GTY(()) another_user_struct {
  enum { TAG1, TAG2 } tag;
  union {
    int GTY((tag("TAG1"))) as_int;
    struct container * GTY((tag("TAG2"))) as_container;
  } GTY((desc("%0.tag"))) u;
} another_user_struct_t;

/* Global variables using our types */
extern struct container GTY(()) main_container;
extern another_user_struct_t GTY(()) global_user_struct;

/* Function declarations that might use these types */
extern void init_my_types(void);
extern void register_callback(callback_func func);

#endif /* MYTEST_H */
