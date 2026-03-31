/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in the gcc/ directory of the GCC source tree */

/* Boilerplate GCC headers */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */
typedef struct OpaqueStruct *OpaquePtr;  /* Pointer to undefined type */

/* Basic scalar types - TYPE_SCALAR */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef enum { RED, GREEN, BLUE } scalar_enum;

/* String type - TYPE_STRING */
const char *string_literal = "test string";
char *dynamic_string GTY((length("strlen(%h) + 1")));

/* Simple struct - TYPE_STRUCT */
struct SimpleStruct {
  int x;
  float y;
};

/* User struct with typedef - TYPE_USER_STRUCT */
typedef struct {
  int id;
  char name[32];
} UserStruct;

/* Another user struct with GTY marker */
typedef struct GTY(()) TaggedUserStruct {
  int tag;
  struct SimpleStruct *GTY((skip)) data;
} TaggedUserStruct;

/* Union type - TYPE_UNION */
union DataUnion {
  int as_int;
  float as_float;
  char as_char[4];
  struct SimpleStruct as_struct;
};

/* Complex nested union with GTY */
union GTY(()) ComplexUnion {
  UserStruct user_data;
  union DataUnion *GTY((tag("0"))) nested_union;
  void (*callback)(int);  /* Function pointer inside union */
};

/* Pointer types - TYPE_POINTER */
int *int_ptr;
float **float_ptr_ptr;
struct SimpleStruct *struct_ptr;
UserStruct *user_struct_ptr;
union DataUnion *union_ptr;

/* Array types - TYPE_ARRAY */
int int_array[10];
float float_array[5][5];  /* Multi-dimensional */
char char_array[] = "incomplete";
struct SimpleStruct struct_array[3];
UserStruct *pointer_array[8];

/* Incomplete array in struct */
struct WithIncompleteArray {
  int count;
  int data[];  /* Flexible array member */
};

/* Function pointer/callback types - TYPE_CALLBACK */
typedef int (*SimpleCallback)(int, int);
typedef void (*ComplexCallback)(struct SimpleStruct *, UserStruct **);

/* Callback returning pointer to array */
typedef int (*CallbackReturningArrayPtr)(void)[10];

/* Callback taking function pointer as parameter */
typedef void (*MetaCallback)(SimpleCallback);

/* Struct containing multiple callbacks */
struct CallbackContainer {
  SimpleCallback func1;
  ComplexCallback func2;
  MetaCallback func3;
};

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY((transaction_safe)) TransactionSafeStruct {
  int value;
  char *GTY((skip)) name;
} __attribute__((aligned(16)));

/* Another language struct with attribute */
struct GTY(()) PackedStruct {
  char a;
  int b;
  char c;
} __attribute__((packed));

/* Complex nested type definitions */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  int count;
  union ComplexUnion *GTY((length("%h.count"))) unions[4];
  UserStruct users[2];
};

/* Union containing struct and callback pointer */
union GTY(()) MixedUnion {
  struct ContainerStruct container;
  SimpleCallback callback;
  struct {
    int type;
    void *GTY((skip)) data;
  } anonymous;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*FuncReturningArrayPtr)(void)[5];

/* Typedef for complex nested pointer */
typedef struct ContainerStruct **DoubleContainerPtr;

/* Even more complex: pointer to array of function pointers */
typedef SimpleCallback (*ArrayOfCallbacksPtr)[3];

/* Struct with all types combined */
struct GTY(()) UltimateStruct {
  /* Scalars */
  int scalar_int;
  float scalar_float;
  
  /* String */
  char *GTY((length("strlen(%h.str) + 1"))) str;
  
  /* Struct */
  struct SimpleStruct simple;
  
  /* User struct */
  UserStruct user;
  
  /* Union */
  union DataUnion data_union;
  
  /* Pointer */
  struct UltimateStruct *GTY((skip)) self_ptr;
  
  /* Array */
  int numbers[7];
  
  /* Callback */
  ComplexCallback callback;
  
  /* Language struct */
  struct TransactionSafeStruct *GTY((skip)) trans_struct;
  
  /* Nested complex types */
  struct ContainerStruct container;
  union MixedUnion mixed;
};

/* Global variables to ensure types are used */
struct SimpleStruct global_simple = {1, 2.0};
UserStruct global_user = {42, "test"};
union DataUnion global_union = {.as_int = 100};
struct ContainerStruct *GTY((skip)) global_container_ptr;
struct UltimateStruct global_ultimate;

/* Function definitions using the types */

void init_ultimate_struct(struct UltimateStruct *us) {
  us->scalar_int = 42;
  us->scalar_float = 3.14;
  us->str = xstrdup("ultimate string");
  us->simple.x = 10;
  us->simple.y = 20.5;
  us->user.id = 99;
  strcpy(us->user.name, "ultimate user");
  us->data_union.as_int = 255;
  us->self_ptr = us;
  
  for (int i = 0; i < 7; i++) {
    us->numbers[i] = i * i;
  }
  
  us->container.count = 2;
  us->mixed.anonymous.type = 1;
}

int sample_callback(int a, int b) {
  return a + b;
}

void complex_callback(struct SimpleStruct *s, UserStruct **u) {
  if (s && u && *u) {
    s->x = (*u)->id;
    s->y = (*u)->id * 2.0;
  }
}

/* Main test function that uses all types */
void GTY_TEST_FUNCTION() {
  /* Use scalar types */
  scalar_int si = 100;
  scalar_float sf = 3.14159;
  (void)si;
  (void)sf;
  
  /* Use string */
  char *local_str = "local string";
  (void)local_str;
  
  /* Use struct */
  struct SimpleStruct local_struct = {5, 6.5};
  UserStruct local_user = {1, "local"};
  
  /* Use union */
  union DataUnion local_union;
  local_union.as_int = 42;
  
  /* Use pointers */
  int *local_int_ptr = &si;
  struct SimpleStruct *local_struct_ptr = &local_struct;
  (void)local_int_ptr;
  (void)local_struct_ptr;
  
  /* Use arrays */
  int local_array[3] = {1, 2, 3};
  struct SimpleStruct local_struct_array[2] = {{1, 2.0}, {3, 4.0}};
  (void)local_array;
  (void)local_struct_array;
  
  /* Use callbacks */
  SimpleCallback cb = sample_callback;
  int result = cb(10, 20);
  (void)result;
  
  ComplexCallback ccb = complex_callback;
  UserStruct *user_ptr = &local_user;
  ccb(&local_struct, &user_ptr);
  
  /* Use language struct */
  struct TransactionSafeStruct trans = {100, "trans"};
  (void)trans;
  
  /* Use complex nested types */
  struct ContainerStruct container;
  container.count = 3;
  
  union MixedUnion mixed;
  mixed.callback = sample_callback;
  
  /* Initialize ultimate struct */
  init_ultimate_struct(&global_ultimate);
}

/* Dummy main to make file compilable */
#ifdef STANDALONE_TEST
int main() {
  GTY_TEST_FUNCTION();
  return 0;
}
#endif
