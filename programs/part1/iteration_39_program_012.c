/* Test header for gengtype coverage - covers all type categories in statistics */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_base_struct {
  int field1;
  my_scalar_t field2;
};

/* TYPE_USER_STRUCT: User-defined struct (often used for templates) */
struct GTY((user)) my_user_struct {
  struct my_base_struct *base;
  int user_data;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  struct my_base_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_base_struct * GTY(()) global_struct_ptr;
extern my_scalar_t * GTY(()) scalar_ptr_array[10];

/* TYPE_ARRAY: Array type */
extern int GTY((length("array_length"))) test_array[];
extern int array_length;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback_fn)(int, const char*);
extern test_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), tag("LANG_STRUCT"))) my_lang_struct {
  enum tree_code type;
  union tree_node * GTY((skip)) node;
  struct my_lang_struct *next;
};

/* Nested structures to ensure thorough processing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) string_field; /* TYPE_STRING */
  struct my_base_struct struct_field; /* TYPE_STRUCT */
  union my_test_union union_field;   /* TYPE_UNION */
  struct my_base_struct *ptr_field;  /* TYPE_POINTER */
  int GTY((length("10"))) array_field[10]; /* TYPE_ARRAY */
  test_callback_fn callback_field;   /* TYPE_CALLBACK */
  struct my_lang_struct *lang_field; /* TYPE_LANG_STRUCT */
};

/* Forward declaration for pointer types */
struct GTY(()) forward_declared_struct;
struct another_struct;

/* Pointer to forward-declared struct */
extern struct forward_declared_struct * GTY(()) forward_ptr;

/* Array of pointers */
typedef struct my_base_struct * GTY(()) base_ptr_array[5];

/* Complex nested example */
struct GTY(()) complex_example {
  base_ptr_array ptrs;               /* TYPE_ARRAY of TYPE_POINTER */
  struct GTY(()) nested {
    int x;
    struct nested *next;
  } *nested_list;                    /* TYPE_POINTER to TYPE_STRUCT */
  
  union GTY(()) variant {
    int i;
    struct nested *n;
    test_callback_fn cb;
  } current_variant;                 /* TYPE_UNION */
};

/* Template-like structure (treated as USER_STRUCT) */
struct GTY((user)) template_struct {
  void * GTY((skip)) data;
  size_t size;
};

#endif /* GCC_MYTEST_H */
