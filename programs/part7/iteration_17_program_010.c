/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct GTY(()) OpaqueStruct;      /* TYPE_UNDEFINED */
union GTY(()) OpaqueUnion;        /* TYPE_UNDEFINED */

/* Scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;
typedef bool GTY(()) scalar_bool;

/* String type - TYPE_STRING */
typedef const char * GTY(()) string_type;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int field1;
  float field2;
};

/* User struct via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
  char * GTY((skip)) name;
} UserStruct;

/* Another struct with complex members */
struct GTY(()) ComplexStruct {
  int * GTY((skip)) int_ptr;
  struct SimpleStruct * GTY((skip)) nested_struct;
  char buffer[100];
};

/* Union type - TYPE_UNION */
union GTY(()) DataUnion {
  int as_int;
  float as_float;
  double as_double;
  char * GTY((skip)) as_string;
  struct SimpleStruct * GTY((skip)) as_struct;
};

/* Pointer types - TYPE_POINTER */
typedef int * GTY((skip)) int_ptr_type;
typedef struct ComplexStruct * GTY((skip)) struct_ptr_type;
typedef union DataUnion * GTY((skip)) union_ptr_type;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_10[10];
typedef char GTY(()) char_array[];
typedef struct SimpleStruct GTY(()) struct_array[5];
typedef int * GTY((skip)) ptr_array[8];

/* Callback/function types - TYPE_CALLBACK */
typedef void (* GTY((skip)) simple_callback)(void);
typedef int (* GTY((skip)) complex_callback)(int, const char *);
typedef struct SimpleStruct * (* GTY((skip)) struct_returning_callback)(void);

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void * GTY((skip)) ptr;
};

/* Another language struct with transaction attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char * GTY((skip)) name;
};

/* Complex nested type definitions */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  union DataUnion * GTY((skip)) union_ptrs[4];
  int count;
  simple_callback cb;
};

/* Union containing struct and callback pointer */
union GTY(()) MixedUnion {
  struct ComplexStruct nested_struct;
  complex_callback cb_func;
  int_array_10 numbers;
};

/* Typedef for function pointer returning pointer to array */
typedef int (* (* GTY((skip)) complex_func_ptr)(void))[10];

/* Even more complex: struct with nested anonymous union */
struct GTY(()) NestedTypes {
  int id;
  union {
    int as_int;
    float as_float;
    struct {
      char * GTY((skip)) name;
      int value;
    } nested;
  } data;
  int (* GTY((skip)) operations[3])(int, int);
};

/* Global variables to instantiate types */
scalar_int global_int GTY((skip));
scalar_float global_float GTY((skip));
string_type global_string GTY((skip)) = "test string";
struct SimpleStruct global_struct GTY((skip));
UserStruct global_user_struct GTY((skip));
union DataUnion global_union GTY((skip));
int_array_10 global_array GTY((skip));
struct LangStruct global_lang_struct GTY((skip));
struct ContainerStruct global_container GTY((skip));

/* Function using various types */
void GTY((skip)) use_types(void) {
  /* Reference undefined types */
  struct OpaqueStruct * GTY((skip)) opaque_ptr = NULL;
  union OpaqueUnion * GTY((skip)) opaque_union_ptr = NULL;
  
  /* Use scalar types */
  scalar_int local_int = 42;
  scalar_float local_float = 3.14f;
  
  /* Use string type */
  string_type local_string = "local string";
  
  /* Use struct types */
  struct SimpleStruct local_struct = {1, 2.0f};
  UserStruct local_user_struct = {10, 20.5, "user"};
  
  /* Use union type */
  union DataUnion local_union;
  local_union.as_int = 100;
  
  /* Use pointer types */
  int_ptr_type int_ptr = &local_int;
  struct_ptr_type struct_ptr = &global_struct;
  
  /* Use array types */
  int_array_10 local_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  char incomplete_array[] = "incomplete";
  
  /* Use callback types */
  simple_callback cb = NULL;
  complex_callback complex_cb = NULL;
  
  /* Use language structs */
  struct LangStruct lang_struct = {42, NULL};
  struct TransactionStruct trans_struct = {100, "transaction"};
  
  /* Use complex nested types */
  struct ContainerStruct container;
  union MixedUnion mixed;
  struct NestedTypes nested;
  
  /* Prevent unused variable warnings */
  (void)opaque_ptr;
  (void)opaque_union_ptr;
  (void)local_int;
  (void)local_float;
  (void)local_string;
  (void)local_struct;
  (void)local_user_struct;
  (void)local_union;
  (void)int_ptr;
  (void)struct_ptr;
  (void)local_array;
  (void)incomplete_array;
  (void)cb;
  (void)complex_cb;
  (void)lang_struct;
  (void)trans_struct;
  (void)container;
  (void)mixed;
  (void)nested;
}

/* Main function for standalone compilation */
int main(int argc, char **argv) {
  use_types();
  return 0;
}

/* Header file simulation */
#ifdef HEADER_FILE
/* In a separate header to test multi-file processing */
#ifndef GTYPE_TEST_HEADER
#define GTYPE_TEST_HEADER

struct GTY(()) HeaderStruct {
  int header_field;
  double header_double;
};

typedef enum GTY(()) {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} HeaderEnum;

union GTY(()) HeaderUnion {
  int i;
  char c;
  struct HeaderStruct * GTY((skip)) s;
};

#endif /* GTYPE_TEST_HEADER */
#endif /* HEADER_FILE */
