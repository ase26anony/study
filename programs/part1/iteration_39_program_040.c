/* Test header with diverse GTY-annotated types for coverage testing */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Simple structure */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: User-defined structure type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  void *ptr_val;
};

/* TYPE_POINTER: Pointer types */
extern struct my_test_struct * GTY(()) my_struct_pointer;
extern my_scalar_t * GTY(()) my_scalar_pointer;
extern union my_test_union * GTY(()) my_union_pointer;

/* TYPE_ARRAY: Array types */
extern int GTY((length("my_array_len"))) my_int_array[];
extern struct my_test_struct GTY(()) my_struct_array[5];
extern my_scalar_t GTY(()) my_scalar_array[10];

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_type {
  int lang_specific1;
  void *lang_specific2;
};

/* Nested/complex types for comprehensive testing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  struct my_test_struct *struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
  int array_field[3];                /* TYPE_ARRAY (implicit) */
  union my_test_union union_field;   /* TYPE_UNION */
  my_callback_fn callback_field;     /* TYPE_CALLBACK */
  const char *string_field;          /* TYPE_STRING */
};

/* Variable declarations using these types */
extern struct complex_container GTY(()) global_container;
extern struct lang_type GTY(()) *lang_struct_ptr;

/* Template-like structure with parameter */
struct GTY(()) template_struct {
  int id;
  void GTY((skip)) *data;  /* Skip annotation for variety */
};

/* Chain structure for pointer testing */
struct GTY(()) chain_node {
  int value;
  struct chain_node GTY((tag("0"))) *next;
  struct chain_node GTY((tag("1"))) *prev;
};

#endif /* MYTEST_GTY_H */
