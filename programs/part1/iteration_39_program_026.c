/* Test header for gengtype coverage - covers all type categories */
#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Simple scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Basic structure type */
struct GTY(()) my_base_struct {
  int field1;
  my_scalar_t field2;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct GTY((user)) my_user_struct {
  void *data;
  int size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_base_struct * GTY(()) my_struct_pointer;
union my_test_union * GTY((tag("0"))) my_union_pointer;

/* TYPE_ARRAY: Array types */
int GTY(()) my_int_array[10];
struct my_base_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), chain_next("%0.next"))) my_lang_struct {
  int type;
  struct my_lang_struct *next;
  union my_test_union data;
};

/* Complex nested type to ensure thorough processing */
struct GTY(()) my_complex_type {
  /* Scalar field */
  my_scalar_t id;
  
  /* String field */
  const char * GTY((skip)) name;
  
  /* Pointer field */
  struct my_lang_struct * GTY((chain_next("%h.next"))) lang_data;
  
  /* Array field */
  my_callback_fn GTY((length("callback_count"))) callbacks[5];
  int callback_count;
  
  /* Union field */
  union my_test_union value;
  
  /* Nested structure */
  struct GTY(()) nested_struct {
    int x;
    int y;
  } point;
};

/* Forward declaration for pointer types */
struct GTY((forward)) my_forward_struct;
struct my_forward_struct * GTY(()) forward_ptr;

/* Variable declarations using our types */
extern struct my_complex_type GTY(()) global_complex_var;
extern my_callback_fn GTY(()) global_callback;

/* Inline function to ensure types are referenced */
static inline void init_test_types(void) {
  /* Reference all types to ensure they're processed */
  my_scalar_t local_scalar = 42;
  (void)local_scalar;
  
  struct my_base_struct local_struct = {1, 2};
  (void)local_struct;
  
  union my_test_union local_union;
  local_union.int_val = 3;
  (void)local_union;
}

#endif /* MYTEST_H */
