/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in gcc/ directory and built with coverage-enabled GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Basic scalar types - TYPE_SCALAR */
int global_int GTY((skip)) = 42;
float global_float GTY((skip)) = 3.14f;
double global_double GTY((skip)) = 2.71828;
char global_char GTY((skip)) = 'A';

/* String type - TYPE_STRING */
const char* GTY((skip)) global_string = "Hello, gengtype!";

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

/* Another struct with GTY markers */
struct GTY(()) ComplexStruct {
  int* GTY((skip)) int_ptr;           /* Pointer type */
  struct SimpleStruct* GTY((skip)) simple_ptr; /* Pointer to struct */
  char data[100];                     /* Array type */
};

/* Union type - TYPE_UNION */
union GTY(()) DataUnion {
  int as_int;
  float as_float;
  char* GTY((skip)) as_string;
  struct SimpleStruct as_struct;
};

/* Array types - TYPE_ARRAY */
int int_array[10] GTY((skip));
struct SimpleStruct struct_array[5] GTY(());
UserStruct user_struct_array[3] GTY(());

/* Pointer types - TYPE_POINTER */
int* GTY((skip)) int_pointer = &global_int;
struct ComplexStruct* GTY((skip)) complex_ptr;
union DataUnion* GTY((skip)) union_ptr;

/* Function pointer types - TYPE_CALLBACK */
typedef int (*CompareFunc)(const void*, const void*);
typedef void (*CallbackFunc)(int, char* GTY((skip)));

/* Struct containing function pointer */
struct GTY(()) CallbackContainer {
  CallbackFunc GTY((skip)) callback;
  CompareFunc GTY((skip)) compare;
  void* GTY((skip)) user_data;
};

/* Complex nested type definitions */
struct GTY(()) SuperComplex {
  /* Array of pointers to unions */
  union DataUnion* GTY((skip)) union_array[8];
  
  /* Pointer to array */
  int (*matrix_ptr)[10][10] GTY((skip));
  
  /* Callback function pointer */
  CallbackFunc GTY((skip)) handlers[4];
  
  /* Nested struct */
  struct GTY(()) Nested {
    int depth;
    struct SuperComplex* GTY((skip)) parent;
  } nested;
  
  /* Union containing struct and callback */
  union GTY(()) {
    struct Nested nested_data;
    CallbackFunc GTY((skip)) alt_handler;
  } variant;
};

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int transaction_id;
  void* GTY((skip)) transaction_data;
} __attribute__((transaction_safe));

/* Another language struct with vector attribute */
struct GTY(()) VectorStruct __attribute__((vector_size(16))) {
  int values[4];
};

/* Incomplete array in struct */
struct GTY(()) FlexibleArray {
  int length;
  char data[];  /* Flexible array member */
};

/* Typedef for complex function pointer */
typedef struct ComplexStruct* (*FactoryFunc)(int, const char* GTY((skip)));

/* Global variables using our types */
struct SimpleStruct simple_instance GTY(()) = {1, 2.0f};
UserStruct user_instance GTY(()) = {100, "test"};
union DataUnion union_instance GTY(()) = {.as_int = 42};
struct ComplexStruct complex_instance GTY(());
struct SuperComplex super_instance GTY(());
struct LangStruct lang_instance GTY(());
struct CallbackContainer callback_instance GTY(());

/* Function definitions that use our types */
static int 
compare_ints(const void* a, const void* b) 
{
  return *(const int*)a - *(const int*)b;
}

static void 
simple_callback(int id, char* GTY((skip)) name) 
{
  /* Do nothing, just for type coverage */
}

/* Function that creates complex type relationships */
void GTY((skip))
create_type_relationships(void)
{
  /* Initialize pointers */
  int_pointer = &global_int;
  complex_ptr = &complex_instance;
  union_ptr = &union_instance;
  
  /* Initialize arrays */
  for (int i = 0; i < 10; i++) {
    int_array[i] = i;
  }
  
  /* Setup callback container */
  callback_instance.callback = simple_callback;
  callback_instance.compare = compare_ints;
  
  /* Create complex nested relationships */
  super_instance.nested.depth = 5;
  super_instance.nested.parent = &super_instance;
  
  /* Use the variant union */
  super_instance.variant.nested_data.depth = 10;
  
  /* Initialize matrix pointer */
  static int matrix[10][10];
  super_instance.matrix_ptr = &matrix;
  
  /* Setup handlers array */
  for (int i = 0; i < 4; i++) {
    super_instance.handlers[i] = simple_callback;
  }
  
  /* Setup union array */
  for (int i = 0; i < 8; i++) {
    super_instance.union_array[i] = &union_instance;
  }
}

/* Main function to ensure everything is referenced */
int
main(int argc, char** argv GTY((skip)))
{
  create_type_relationships();
  
  /* Use all global instances to prevent optimization */
  if (simple_instance.x > 0) {
    user_instance.id = simple_instance.x;
  }
  
  if (union_instance.as_int > 0) {
    lang_instance.transaction_id = union_instance.as_int;
  }
  
  /* Exercise callback */
  if (callback_instance.callback) {
    callback_instance.callback(1, "test");
  }
  
  return 0;
}

/* Additional forward declarations to ensure TYPE_UNDEFINED coverage */
struct AnotherOpaque;
typedef struct YetAnotherOpaque YetAnotherOpaque;
union ForwardUnion;
