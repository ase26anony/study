/* Test header with GTY annotations to cover all type categories in gengtype.cc */
/* This should be placed in the gcc/ directory of the GCC source tree */

#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_type field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(struct my_user_struct *);
};

/* TYPE_UNION: Union type with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char *string_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Pointer type with GTY annotation */
typedef struct my_test_struct * GTY(()) my_struct_pointer;

/* TYPE_ARRAY: Array type with GTY annotation */
extern int GTY((length("my_array_length"))) my_test_array[];
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*GTY(()) my_callback_func)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) my_lang_struct {
  int lang_specific_data;
  tree decl;  /* Assuming tree is defined in context */
};
#endif

/* Complex nested example to ensure thorough processing */
struct GTY(()) container_struct {
  /* Multiple pointer types */
  struct my_test_struct * GTY(()) first;
  union my_test_union * GTY(()) second;
  
  /* Array of pointers */
  my_callback_func GTY((length("callback_count"))) callbacks[];
  int callback_count;
  
  /* Nested struct */
  struct GTY(()) nested {
    my_scalar_type value;
    const char * GTY(()) name;
  } nested_item;
  
  /* Union field */
  union my_test_union data;
};

/* Another example with skip marker */
struct GTY((skip)) skipped_struct {
  void *opaque_data;  /* Will be skipped in GC */
  int regular_field;
};

/* Chain structure for linked list testing */
struct GTY(()) chain_node {
  int id;
  const char * GTY(()) label;
  struct chain_node * GTY(()) next;
  struct chain_node * GTY(()) prev;
};

/* Test variable declarations using these types */
extern struct my_test_struct GTY(()) global_test_struct;
extern union my_test_union GTY(()) global_test_union;
extern my_struct_pointer GTY(()) global_pointer_array[5];
extern my_callback_func GTY(()) global_callback;

#endif /* MYTEST_GTY_H */
