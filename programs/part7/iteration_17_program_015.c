/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in the gcc/ directory and built as part of GCC */

/* Boilerplate GCC headers */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Basic scalar types - TYPE_SCALAR */
typedef int my_int;           /* TYPE_SCALAR */
typedef float my_float;       /* TYPE_SCALAR */
typedef double my_double;     /* TYPE_SCALAR */
typedef char my_char;         /* TYPE_SCALAR */
typedef bool my_bool;         /* TYPE_SCALAR */

/* String type - TYPE_STRING */
typedef const char *my_string; /* TYPE_STRING */

/* Simple struct - TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int field1;
  float field2;
};

/* User struct via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
} UserStruct;

/* Another user struct with complex members */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct *GTY((skip)) us_ptr;
  int data[5];
} ComplexUserStruct;

/* Union type - TYPE_UNION */
union GTY(()) MyUnion {
  int as_int;
  float as_float;
  void *GTY((skip)) as_ptr;
};

/* Pointer types - TYPE_POINTER */
typedef int *int_ptr;                    /* TYPE_POINTER */
typedef SimpleStruct *struct_ptr;        /* TYPE_POINTER */
typedef void (*func_ptr)(void);          /* TYPE_POINTER (to function) */

/* Array types - TYPE_ARRAY */
typedef int int_array[10];               /* TYPE_ARRAY */
typedef SimpleStruct struct_array[5];    /* TYPE_ARRAY */

/* Incomplete array */
struct GTY(()) WithIncompleteArray {
  int count;
  int data[];  /* TYPE_ARRAY (incomplete) */
};

/* Callback/function types - TYPE_CALLBACK */
typedef int (*BinaryOp)(int, int);       /* TYPE_CALLBACK */
typedef void (*CallbackFunc)(void *);    /* TYPE_CALLBACK */

/* Complex callback returning pointer to array */
typedef int (*ComplexCallback)(void)[10]; /* TYPE_CALLBACK */

/* Language-specific struct - TYPE_LANG_STRUCT */
/* Use GCC attributes to potentially trigger this type */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void *GTY((skip)) ptr;
};

/* Another language struct with transaction_safe attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char *GTY((skip)) name;
};

/* Now create complex nested types to ensure thorough traversal */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  int id;
  MyUnion *GTY((skip)) union_array[8];  /* Array of pointers to union */
  BinaryOp operation;                   /* Callback type */
};

/* Union containing struct and callback pointer */
union GTY(()) MegaUnion {
  ContainerStruct as_struct;
  CallbackFunc callback;
  int_array as_int_array;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*FuncReturningArrayPtr)(void)[5];

/* Struct with all kinds of members */
struct GTY(()) UltimateStruct {
  /* Scalar members */
  my_int scalar1;
  my_float scalar2;
  
  /* String member */
  my_string str;
  
  /* Struct members */
  SimpleStruct simple;
  UserStruct user;
  
  /* Union member */
  MyUnion uni;
  
  /* Pointer members */
  int_ptr int_pointer;
  struct_ptr struct_pointer;
  
  /* Array members */
  int_array fixed_array;
  struct_array struct_array_field;
  
  /* Callback members */
  BinaryOp bin_op;
  CallbackFunc cb;
  
  /* Language struct */
  LangStruct lang;
  
  /* Pointer to undefined type */
  struct OpaqueStruct *GTY((skip)) opaque_ptr;
  
  /* Nested complex type */
  MegaUnion nested_union;
};

/* Function pointer type definitions */
typedef void (*SimpleHandler)(void);
typedef int (*Processor)(UltimateStruct *GTY((skip)), int);

/* Global variables to instantiate types */
SimpleStruct GTY((skip)) global_struct = {1, 2.0f};
UserStruct GTY((skip)) global_user_struct = {10, 20.5};
MyUnion GTY((skip)) global_union = {.as_int = 42};
ContainerStruct GTY((skip)) global_container;
UltimateStruct GTY((skip)) global_ultimate;

/* Dummy function using the types */
void GTY((skip)) gt_test_function(void)
{
  /* Use scalar types */
  my_int x = 10;
  my_float y = 3.14f;
  my_string s = "test string";  /* TYPE_STRING */
  
  /* Use struct types */
  SimpleStruct local_simple = {x, y};
  UserStruct local_user = {5, 10.5};
  
  /* Use union */
  MyUnion local_union;
  local_union.as_int = 100;
  
  /* Use pointers */
  int_ptr p = &x;
  struct_ptr sp = &local_simple;
  
  /* Use arrays */
  int_array arr = {0,1,2,3,4,5,6,7,8,9};
  struct_array sarr;
  
  /* Use callbacks */
  BinaryOp adder = NULL;
  
  /* Use language struct */
  LangStruct ls = {99, NULL};
  
  /* Reference undefined types */
  extern struct OpaqueStruct *get_opaque(void);
  struct OpaqueStruct *opaque = get_opaque();
  
  /* Complex nested access */
  global_container.union_array[0] = &local_union;
  global_ultimate.nested_union.as_struct.id = 123;
  
  /* Prevent unused variable warnings */
  (void)s;
  (void)local_user;
  (void)sp;
  (void)arr;
  (void)sarr;
  (void)adder;
  (void)ls;
  (void)opaque;
}

/* Another function with different type usage patterns */
Processor GTY((skip)) get_processor(void)
{
  static int process_impl(UltimateStruct *GTY((skip)) us, int val)
  {
    return us->scalar1 + val;
  }
  return process_impl;
}

/* Main function for source file validity */
int main(int argc, char **argv)
{
  gt_test_function();
  
  /* Create instances of various types */
  UltimateStruct local_ultimate = {
    .scalar1 = 1,
    .scalar2 = 2.0f,
    .str = "Hello",
    .simple = {1, 2.0f},
    .user = {3, 4.0},
    .uni = {.as_int = 5},
    .int_pointer = &local_ultimate.scalar1,
    .struct_pointer = &local_ultimate.simple,
    .fixed_array = {0},
    .struct_array_field = {{0,0}},
    .bin_op = NULL,
    .cb = NULL,
    .lang = {6, NULL},
    .opaque_ptr = NULL,
    .nested_union = {.as_struct = {7, {NULL}, NULL}}
  };
  
  /* Use array of pointers */
  void *GTY((skip)) ptr_array[4];
  ptr_array[0] = &local_ultimate;
  ptr_array[1] = &global_struct;
  ptr_array[2] = &global_union;
  ptr_array[3] = NULL;
  
  return 0;
}

/* Additional type in different style */
typedef enum GTY(()) {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} MyEnum;

/* Struct with bitfields */
struct GTY(()) BitfieldStruct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Template-like pattern using macros */
#define DECLARE_HANDLER(name) \
  typedef void (*name##_handler)(int); \
  struct GTY(()) name##_record { \
    name##_handler h; \
    int data; \
  }

DECLARE_HANDLER(my);
DECLARE_HANDLER(your);

/* Force inclusion of all types in type graph */
static union {
  SimpleStruct a;
  UserStruct b;
  ComplexUserStruct c;
  MyUnion d;
  ContainerStruct e;
  UltimateStruct f;
  LangStruct g;
  TransactionStruct h;
  BitfieldStruct i;
  my_record j;
  your_record k;
} GTY((skip)) type_union_all;
