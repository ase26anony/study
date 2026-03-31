/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in gcc/ directory and built with coverage-enabled GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Basic scalar types - TYPE_SCALAR */
typedef int my_int;
typedef float my_float;
typedef double my_double;
typedef char my_char;
typedef bool my_bool;

/* String type - TYPE_STRING */
typedef const char *my_string;

/* Simple struct - TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int field1;
  float field2;
};

/* User struct with typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
  char * GTY((skip)) name;
} UserStruct;

/* Another user struct */
typedef struct GTY(()) {
  UserStruct* GTY((tag("0"))) us_ptr;
  int counter;
} ContainerStruct;

/* Union type - TYPE_UNION */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char* GTY((skip)) string_val;
};

/* Pointer types - TYPE_POINTER */
typedef int* int_ptr;
typedef SimpleStruct* struct_ptr;
typedef void (*func_ptr)(void);

/* Array types - TYPE_ARRAY */
typedef int int_array[10];
typedef SimpleStruct struct_array[5];
extern int incomplete_array[];  /* Incomplete array type */

/* Complex nested array */
typedef int* pointer_array[20];

/* Callback/function types - TYPE_CALLBACK */
typedef int (*comparator_fn)(const void*, const void*);
typedef void (*callback_fn)(int, char*);
typedef UserStruct* (*factory_fn)(void);

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void* GTY((skip)) ptr;
};

/* Another language struct with transaction attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char buffer[256];
};

/* Complex nested type combining multiple categories */
struct GTY(()) ComplexType {
  /* Scalar members */
  int id;
  float score;
  
  /* Pointer member */
  ComplexType* GTY((tag("1"))) next;
  
  /* Array member */
  int values[8];
  
  /* Pointer to array */
  int* GTY((skip)) dynamic_array;
  
  /* Union member */
  DataUnion data;
  
  /* Callback pointer */
  comparator_fn compare;
  
  /* Nested struct */
  struct GTY(()) {
    int x;
    int y;
  } position;
  
  /* String pointer */
  const char* GTY((skip)) description;
};

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  UserStruct user_data;
  callback_fn handler;
  int number;
};

/* Array of pointers to unions */
MixedUnion* GTY((skip)) union_array[15];

/* Function pointer returning pointer to array */
typedef int (*array_getter_fn)(void)[10];

/* Struct with array of function pointers */
struct GTY(()) CallbackContainer {
  callback_fn handlers[5];
  factory_fn factories[3];
};

/* More complex nesting */
struct GTY(()) SuperNested {
  /* Struct containing array of pointers to unions */
  MixedUnion* GTY((skip)) mixed_ptrs[7];
  
  /* Union containing struct and callback pointer */
  union GTY(()) {
    ComplexType complex;
    func_ptr function;
  } choice;
  
  /* Pointer to language struct */
  LangStruct* GTY((skip)) lang_ptr;
  
  /* Array of structs */
  SimpleStruct simple_array[3];
};

/* Global variables to ensure types are used */
SimpleStruct GTY((skip)) global_struct;
UserStruct* GTY((skip)) global_user_struct = NULL;
DataUnion GTY((skip)) global_union;
ComplexType* GTY((skip)) global_complex = NULL;
LangStruct GTY((skip)) global_lang_struct;
TransactionStruct GTY((skip)) global_transaction_struct;

/* Function using the types */
void GTY((skip)) initialize_types(void) {
  /* Create instances */
  static SimpleStruct local_struct = {1, 2.0f};
  static UserStruct local_user = {10, 3.14, "test"};
  static DataUnion local_union;
  static ComplexType local_complex;
  static LangStruct local_lang;
  static TransactionStruct local_transaction;
  
  /* Use pointers */
  int_ptr ptr = &local_struct.field1;
  struct_ptr sptr = &local_struct;
  
  /* Use arrays */
  int_array arr = {0};
  struct_array sarr;
  
  /* Use callbacks */
  comparator_fn cmp = NULL;
  callback_fn cb = NULL;
  
  /* Reference undefined types */
  extern struct OpaqueStruct* get_opaque(void);
  extern union OpaqueUnion* get_opaque_union(void);
  
  /* Avoid unused variable warnings */
  (void)ptr;
  (void)sptr;
  (void)arr;
  (void)sarr;
  (void)cmp;
  (void)cb;
}

/* Another function with more complex type usage */
void GTY((skip)) complex_type_operations(void) {
  /* Nested type access */
  ComplexType ct;
  ct.id = 100;
  ct.score = 95.5f;
  ct.next = &ct;  /* Self-reference */
  ct.compare = NULL;
  ct.description = "Complex type instance";
  
  /* Array of pointers access */
  for (int i = 0; i < 7; i++) {
    if (global_complex && global_complex->mixed_ptrs[i]) {
      /* Access union array */
    }
  }
  
  /* Language struct usage */
  global_lang_struct.data = 42;
  global_transaction_struct.value = 100;
  
  /* Callback container */
  CallbackContainer cc;
  for (int i = 0; i < 5; i++) {
    cc.handlers[i] = NULL;
  }
}

/* Main function to ensure everything is referenced */
int main(int argc, char** argv) {
  initialize_types();
  complex_type_operations();
  
  /* Reference global variables */
  global_struct.field1 = argc;
  if (global_user_struct) {
    global_user_struct->x = 10;
  }
  global_union.int_val = 42;
  
  return 0;
}

/* Additional type definitions in different "context" */

/* Simulate multi-file scenario through included header */
#include "gtype-test-aux.h"

/* More pointer variations */
typedef void (*void_func_ptr)(void);
typedef int* (*int_ptr_getter)(void);
typedef UserStruct** double_ptr_to_user_struct;

/* Array of function pointers returning different types */
typedef void* (*void_ptr_getters[5])(void);
typedef int (*int_returners[3])(float, double);

/* Complete the undefined types */
struct OpaqueStruct {
  int hidden;
  void* GTY((skip)) secret;
};

union OpaqueUnion {
  long big;
  double precise;
  void* GTY((skip)) pointer;
};
