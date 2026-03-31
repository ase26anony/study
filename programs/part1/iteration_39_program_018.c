/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for variety */
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  const char * GTY((skip)) string_val;
  struct my_test_struct * GTY((skip)) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) my_struct_pointer;
extern my_scalar_t * GTY(()) my_scalar_pointer;
extern union my_test_union * GTY(()) my_union_pointer;

/* TYPE_ARRAY: Array types with GTY annotations */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];
extern const char * GTY((length("strlen(%h)"))) string_array[20];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
/* This will be processed by gengtype */
struct GTY(()) lang_decl {
  int lang_specific;
  const char * GTY((skip)) lang_name;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) container_struct {
  /* Scalar */
  my_scalar_t count;
  
  /* String */
  const char * GTY(()) description;
  
  /* Pointer */
  struct my_test_struct * GTY(()) data;
  
  /* Array */
  int GTY(()) values[8];
  
  /* Union */
  union my_test_union GTY(()) variant;
  
  /* Callback */
  my_callback_fn GTY(()) handler;
  
  /* Nested struct */
  struct GTY(()) nested {
    int id;
    const char * GTY((skip)) tag;
  } inner;
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;
struct GTY(()) linked_node {
  int value;
  struct forward_declared * GTY(()) next;
};

struct GTY(()) forward_declared {
  int data;
  struct linked_node * GTY(()) prev;
};

/* Template-like macro usage (common in GCC) */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct GTY(()) name { \
    field_type GTY(()) field; \
    struct name * GTY(()) next; \
  }

DEFINE_GTY_STRUCT(gty_list_int, int);
DEFINE_GTY_STRUCT(gty_list_ptr, struct my_test_struct*);

/* Variable declarations using our types */
extern my_scalar_t GTY(()) global_scalar;
extern struct container_struct GTY(()) global_container;
extern struct gty_list_int * GTY(()) int_list_head;

#endif /* GCC_MYTEST_H */
