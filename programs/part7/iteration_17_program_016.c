/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"

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
const char* GTY(()) global_string = "Hello, gengtype!";

/* Simple struct - TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int x;
  float y;
};

/* User struct via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int id;
  char name[32];
} UserStruct;

/* Another struct with complex members */
struct GTY(()) ComplexStruct {
  int* GTY((skip)) int_ptr;      /* Pointer (skip from GC) */
  struct SimpleStruct* GTY(()) nested_struct;
  char buffer[100];
};

/* Union type - TYPE_UNION */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char* GTY((skip)) char_ptr;
};

/* Pointer types - TYPE_POINTER */
int* GTY((skip)) global_int_ptr;
struct ComplexStruct* GTY(()) global_struct_ptr;
union DataUnion* GTY(()) global_union_ptr;

/* Array types - TYPE_ARRAY */
int GTY(()) global_int_array[10];
struct SimpleStruct GTY(()) struct_array[5];
extern int GTY(()) incomplete_array[];  /* Incomplete array */

/* Callback/function types - TYPE_CALLBACK */
typedef int (*CompareFunc)(const void*, const void*);
typedef void (*Callback)(int, char* GTY((skip)));

/* Function pointer with complex return type */
struct ComplexStruct* (*GTY(()) complex_func_ptr)(int, char**);

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void* GTY((skip)) opaque;
};

/* Another language struct with transaction attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char* GTY((skip)) description;
};

/* Nested type combinations */
struct GTY(()) Container {
  /* Array of pointers to unions */
  union DataUnion* GTY(()) union_ptrs[8];
  
  /* Pointer to array of structs */
  struct SimpleStruct (*GTY(()) struct_array_ptr)[5];
  
  /* Callback member */
  CompareFunc GTY((skip)) comparator;
  
  /* Nested union containing struct and callback */
  union GTY(()) {
    struct SimpleStruct nested_struct;
    Callback callback_func;
  } choice;
};

/* Typedef for complex function pointer */
typedef struct ComplexStruct* (*FactoryFunc)(int, char* GTY((skip)));

/* Global instances to ensure types are used */
struct SimpleStruct GTY(()) g_simple = {1, 2.0f};
UserStruct GTY(()) g_user = {42, "test"};
union DataUnion GTY(()) g_union = {.int_val = 100};
struct Container GTY(()) g_container;

/* Function using various types */
void GTY((skip)) use_types() {
  /* Use scalar types */
  my_int i = 10;
  my_float f = 3.14f;
  my_double d = 2.71828;
  my_char c = 'A';
  my_bool b = true;
  
  /* Use string */
  const char* local_str = "local string";
  
  /* Use structs */
  struct SimpleStruct local_struct = {2, 4.5f};
  UserStruct local_user = {99, "local"};
  
  /* Use pointers */
  int* local_int_ptr = &i;
  struct ComplexStruct* local_complex = 0;
  
  /* Use arrays */
  int local_array[3] = {1, 2, 3};
  
  /* Use union */
  union DataUnion local_union;
  local_union.float_val = 1.5f;
  
  /* Use callback */
  CompareFunc local_cmp = 0;
  
  /* Prevent unused variable warnings */
  (void)i; (void)f; (void)d; (void)c; (void)b;
  (void)local_str; (void)local_struct; (void)local_user;
  (void)local_int_ptr; (void)local_complex; (void)local_array;
  (void)local_union; (void)local_cmp;
}

/* Another function with complex type usage */
FactoryFunc GTY((skip)) get_factory() {
  static struct ComplexStruct GTY(()) factory_data = {0, 0, ""};
  
  /* Function returning function pointer */
  return 0;
}

/* Main function for standalone compilation */
#ifdef STANDALONE_TEST
int main() {
  use_types();
  get_factory();
  return 0;
}
#endif

/* Additional undefined type references */
struct OpaqueStruct* GTY((skip)) opaque_ptr;
union OpaqueUnion* GTY((skip)) opaque_union_ptr;

/* Array of function pointers */
Callback GTY((skip)) callbacks[3];

/* Multi-dimensional array */
int GTY(()) matrix[3][4];

/* Struct with bitfield */
struct GTY(()) BitfieldStruct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
};

/* Anonymous struct within union */
union GTY(()) AnonUnion {
  struct {
    int x;
    int y;
  } point;
  int coordinates[2];
};

/* Const pointer to const data */
const struct SimpleStruct* const GTY(()) const_ptr = &g_simple;

/* Volatile qualified type */
volatile int GTY(()) volatile_counter = 0;

/* Restrict qualified pointer */
int* GTY((skip)) restrict GTY((skip)) restricted_ptr;

/* Atomic type (C11) */
_Atomic int GTY(()) atomic_counter = 0;

/* Aligned type */
int GTY(()) aligned_data __attribute__((aligned(64)));

/* Packed struct */
struct GTY(()) __attribute__((packed)) PackedStruct {
  char a;
  int b;
  char c;
};

/* Transparent union GCC extension */
typedef union GTY(()) __attribute__((transparent_union)) TransparentUnion {
  int* GTY((skip)) int_ptr;
  void* GTY((skip)) void_ptr;
} TransparentUnion;

/* Vector type (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* Struct with flexible array member */
struct GTY(()) FlexStruct {
  int count;
  int data[];
};

/* Final global to reference everything */
struct GTY(()) TypeCollection {
  struct SimpleStruct simple;
  UserStruct user;
  union DataUnion data_union;
  struct Container container;
  struct LangStruct lang;
  struct TransactionStruct transaction;
  struct BitfieldStruct bits;
  union AnonUnion anon;
  struct PackedStruct packed;
  int* GTY((skip)) ptr_array[5];
  Callback GTY((skip)) func_array[2];
};

/* Ensure TypeCollection is instantiated */
struct TypeCollection GTY(()) g_collection;
