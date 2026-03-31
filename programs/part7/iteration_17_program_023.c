/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Scalar types - TYPE_SCALAR */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef bool scalar_bool;

/* String type - TYPE_STRING */
typedef const char *string_type;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct (typedef struct) - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
} UserStruct;

/* Another user struct with more complexity */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct *GTY((tag("0"))) us_ptr;
  int data[5];
} ComplexUserStruct;

/* Union type - TYPE_UNION */
union GTY(()) TestUnion {
  int as_int;
  float as_float;
  BasicStruct *GTY((skip)) as_struct_ptr;
};

/* Pointer types - TYPE_POINTER */
typedef int *int_ptr;
typedef BasicStruct *struct_ptr;
typedef void *void_ptr;

/* Array types - TYPE_ARRAY */
typedef int int_array[10];
typedef float float_array[];
typedef BasicStruct struct_array[5];

/* Callback/function types - TYPE_CALLBACK */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, float, BasicStruct*);
typedef void (*callback_returning_ptr)(int**);

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void *GTY((skip)) ptr;
};

/* Another language struct with transaction attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  UserStruct user;
};

/* Complex nested type definitions */

/* Struct containing array of pointers */
struct GTY(()) ContainerStruct {
  int_ptr ptr_array[8];
  float_array dynamic_floats;
  int count;
};

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  struct GTY(()) {
    int tag;
    UserStruct data;
  } s;
  complex_callback cb;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*(*complex_func_ptr)(void))[5];

/* Struct with all kinds of members */
struct GTY(()) MasterStruct {
  /* Scalar */
  int id;
  float score;
  
  /* String */
  const char *GTY((skip)) name;
  
  /* Struct */
  BasicStruct basic;
  
  /* User struct */
  UserStruct user;
  
  /* Union */
  TestUnion union_data;
  
  /* Pointer */
  ContainerStruct *GTY((tag("1"))) container;
  
  /* Array */
  int numbers[20];
  
  /* Callback */
  simple_callback notify;
  
  /* Language struct */
  LangStruct lang_data;
  
  /* Undefined type pointer */
  struct OpaqueStruct *GTY((skip)) opaque;
};

/* Array of unions */
union GTY(()) UnionArrayElement {
  int i;
  float f;
  char *GTY((skip)) str;
};

typedef union GTY(()) UnionArrayElement UnionArray[10];

/* Callback that uses multiple complex types */
typedef MasterStruct* (*factory_callback)(int, UserStruct*, UnionArray);

/* Global variables to ensure types are used */
BasicStruct GTY((root)) global_basic_struct = {0};
UserStruct GTY((root)) global_user_struct = {0};
MasterStruct GTY((root)) global_master_struct = {0};
int_ptr GTY((root)) global_int_ptr = NULL;
UnionArray GTY((root)) global_union_array = {0};

/* Function using the complex types */
void GTY((user)) gt_test_function(void)
{
  /* Create instances and use pointers */
  static BasicStruct local_basic = {1, 2.0f};
  static UserStruct local_user = {3, 4.0};
  static TestUnion local_union;
  static ContainerStruct local_container;
  static MasterStruct local_master;
  
  /* Use string type */
  const char *message = "Test string for TYPE_STRING";
  
  /* Use array types */
  int_array fixed_array = {0};
  float dynamic_floats[] = {1.0f, 2.0f, 3.0f};
  
  /* Use callback */
  simple_callback cb = NULL;
  
  /* Reference undefined types */
  extern struct OpaqueStruct *get_opaque(void);
  struct OpaqueStruct *opaque_ptr = get_opaque();
  
  /* Complex nesting */
  local_master.container = &local_container;
  local_union.as_struct_ptr = &local_basic;
  
  /* Prevent unused variable warnings */
  (void)message;
  (void)fixed_array;
  (void)dynamic_floats;
  (void)cb;
  (void)opaque_ptr;
}

/* Another function with different type usage patterns */
void GTY((user)) process_complex_types(
    ComplexUserStruct *GTY((skip)) cus,
    factory_callback create)
{
  if (cus && create) {
    UnionArray arr = {0};
    MasterStruct *ms = create(42, &cus->us_ptr ? *cus->us_ptr : global_user_struct, arr);
    if (ms) {
      ms->notify();
    }
  }
}

/* Main function to ensure the file compiles */
int main(int argc, char **argv)
{
  gt_test_function();
  
  /* Reference global variables */
  global_basic_struct.field1 = argc;
  global_user_struct.x = argc * 2;
  
  return 0;
}

/* Include a header with more type declarations */
#include "gtype-test-headers.h"
