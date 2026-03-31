/* Test header for gengtype coverage - contains various GTY-annotated types */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct types */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for testing */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) my_user_struct {
  int data;
  void (*marker)(void *);
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((tag("0"))) str_val;  /* tag for union discrimination */
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) my_struct_pointer;
extern union my_test_union * GTY((length("my_union_length"))) my_union_array_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char *);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), chain_next("%0.next"))) lang_struct {
  enum tree_code type;
  struct lang_struct *next;
  tree GTY((skip)) lang_tree;  /* Using GCC's tree type */
};

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  struct my_test_struct *struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
  int GTY(()) matrix[3][3];          /* TYPE_ARRAY (multi-dimensional) */
  union my_test_union data_union;    /* TYPE_UNION */
  void (*GTY(()) operations[5])(void); /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* Variable declarations using our types */
extern struct my_test_struct GTY(()) global_test_struct;
extern union my_test_union GTY(()) global_test_union;
extern struct complex_container * GTY(()) container_ptr;

/* Function prototypes */
extern void init_test_types(void);
extern void register_callback(my_callback_fn fn);

#endif /* GCC_MYTEST_H */
