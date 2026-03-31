/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_t field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct (often used with tag) */
typedef struct GTY((tag("USER_STRUCT"))) my_user_struct {
  int data;
  void *ptr;
} my_user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char *string_val;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern my_scalar_t * GTY(()) scalar_ptr;
extern union my_test_union * GTY(()) union_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
extern struct my_test_struct GTY(()) struct_array[5];
extern char * GTY(()) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback_fn)(int, const char*);
extern test_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_decl {
  int lang_specific;
  void *data;
};

/* Forward declarations for pointer types */
struct forward_declared;
extern struct forward_declared * GTY(()) forward_ptr;

/* Complex nested example covering multiple types */
struct GTY(()) complex_type {
  /* Scalar */
  int count;
  
  /* String */
  const char * GTY(()) name;
  
  /* Pointer to struct */
  struct complex_type * GTY(()) next;
  
  /* Array */
  int GTY(()) values[8];
  
  /* Union */
  union my_test_union GTY(()) data;
  
  /* Callback */
  test_callback_fn GTY(()) handler;
  
  /* Pointer array */
  struct my_test_struct * GTY(()) items[4];
};

/* Template for parameterized types (simulated) */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct GTY(()) name { \
    field_type value; \
    struct name *next; \
  }

DEFINE_GTY_STRUCT(int_list, int);
DEFINE_GTY_STRUCT(ptr_list, void*);

/* Enumeration (treated as scalar for GTY purposes) */
typedef enum GTY(()) test_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} test_enum_t;

/* Multiple inheritance-like structure using unions */
struct GTY(()) base_struct {
  int base_field;
};

struct GTY(()) derived_struct {
  struct base_struct base;
  int derived_field;
};

/* Null pointer type case */
extern void * GTY(()) null_ptr;

/* Opaque pointer type */
typedef struct opaque * GTY(()) opaque_ptr_t;

#endif /* MYTEST_H */
