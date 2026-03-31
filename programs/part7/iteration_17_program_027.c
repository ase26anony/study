/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for TYPE_UNDEFINED */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* TYPE_SCALAR declarations */
typedef int scalar_int;       /* TYPE_SCALAR */
typedef float scalar_float;   /* TYPE_SCALAR */
typedef double scalar_double; /* TYPE_SCALAR */
typedef char scalar_char;     /* TYPE_SCALAR */

/* TYPE_STRING - char* used in string context */
const char* GTY(()) string_literal = "test string";

/* Basic struct for TYPE_STRUCT */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct for TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
} UserStruct;

/* Another user struct with typedef */
typedef struct GTY(()) {
  char* GTY((skip)) name;
  int id;
} NamedUserStruct;

/* TYPE_UNION */
union GTY(()) TestUnion {
  int as_int;
  float as_float;
  struct BasicStruct* GTY((skip)) as_struct_ptr;
};

/* TYPE_POINTER variations */
int* GTY((skip)) global_int_ptr;
struct BasicStruct* GTY((skip)) global_struct_ptr;
UserStruct* GTY((skip)) global_user_struct_ptr;

/* TYPE_ARRAY variations */
int GTY(()) int_array[10];                    /* Fixed size array */
int GTY(()) int_incomplete_array[];           /* Incomplete array */
struct BasicStruct GTY(()) struct_array[5];   /* Array of structs */
UserStruct* GTY((skip)) ptr_array[8];         /* Array of pointers */

/* TYPE_CALLBACK - Function pointer types */
typedef int (*SimpleCallback)(int, float);    /* Function pointer type */
typedef void (*ComplexCallback)(struct BasicStruct*, UserStruct*);

/* Actual function pointer variable */
SimpleCallback GTY((skip)) global_callback;

/* Complex nested type definitions */

/* Struct containing array of pointers to union */
struct GTY(()) ContainerStruct {
  union TestUnion* GTY((skip)) union_ptrs[4];
  int count;
};

/* Union containing struct and callback pointer */
union GTY(()) ComplexUnion {
  struct ContainerStruct container;
  ComplexCallback callback_func;
  void* GTY((skip)) data;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*CallbackReturningArrayPtr)(void)[10];

/* Struct with nested complex types */
struct GTY(()) MasterStruct {
  struct ContainerStruct nested_container;
  union ComplexUnion nested_union;
  SimpleCallback nested_callback;
  int* GTY((skip)) dynamic_array;
  int array_2d[3][4];  /* Multi-dimensional array */
};

/* Language-specific struct with GCC attributes for TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int special_field;
  void* GTY((skip)) lang_specific_ptr;
};

/* Another language struct with transaction attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int transaction_id;
  struct MasterStruct* GTY((skip)) related_struct;
};

/* Global variables using our complex types */
struct MasterStruct GTY(()) global_master;
union ComplexUnion GTY(()) global_complex_union;
struct LangStruct GTY(()) global_lang_struct;

/* Function that uses all types to prevent dead code elimination */
void GTY((skip)) gt_test_function(void)
{
  /* Use scalar types */
  scalar_int si = 42;
  scalar_float sf = 3.14f;
  scalar_double sd = 2.71828;
  scalar_char sc = 'A';
  
  /* Use string */
  const char* local_string = "local string";
  
  /* Instantiate structs */
  struct BasicStruct bs = {1, 2.0f};
  UserStruct us = {10, 20.5};
  
  /* Use union */
  union TestUnion tu;
  tu.as_int = 100;
  
  /* Use pointers */
  int* local_int_ptr = &si;
  struct BasicStruct* local_struct_ptr = &bs;
  
  /* Use arrays */
  int local_array[5] = {1, 2, 3, 4, 5};
  struct BasicStruct local_struct_array[2];
  
  /* Use callback */
  if (global_callback) {
    global_callback(5, 3.14f);
  }
  
  /* Use complex nested types */
  global_master.nested_container.count = 3;
  global_complex_union.callback_func = 0;
  global_lang_struct.special_field = 99;
  
  /* Reference undefined types (forward declarations) */
  struct OpaqueStruct* opaque_ptr = 0;
  union OpaqueUnion* opaque_union_ptr = 0;
  
  /* Prevent unused variable warnings */
  (void)si; (void)sf; (void)sd; (void)sc;
  (void)local_string;
  (void)bs; (void)us;
  (void)tu;
  (void)local_int_ptr; (void)local_struct_ptr;
  (void)local_array; (void)local_struct_array;
  (void)opaque_ptr; (void)opaque_union_ptr;
}

/* Additional complex typedefs to ensure full coverage */
typedef struct GTY(()) {
  union ComplexUnion data;
  CallbackReturningArrayPtr array_getter;
} TypedefWithCallback;

typedef int (*NestedCallback)(struct ContainerStruct*, union ComplexUnion*);

/* Struct with function pointer array */
struct GTY(()) StructWithCallbackArray {
  SimpleCallback callbacks[3];
  NestedCallback nested;
};

/* Main function for standalone compilation */
int main(int argc, char** argv)
{
  gt_test_function();
  return 0;
}
