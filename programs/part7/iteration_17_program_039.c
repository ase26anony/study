/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Forward declarations for TYPE_UNDEFINED */
struct GTY(()) OpaqueStruct;      /* Undefined struct */
union GTY(()) OpaqueUnion;        /* Undefined union */

/* TYPE_SCALAR declarations */
int GTY(()) global_int;           /* Scalar type */
float GTY(()) global_float;       /* Scalar type */
double GTY(()) global_double;     /* Scalar type */
char GTY(()) global_char;         /* Scalar type */

/* TYPE_STRING - char* with string context */
const char* GTY(()) global_string = "test string";

/* Simple struct for TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int x;
  float y;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct GTY(()) {
  int id;
  char name[32];
} UserStruct;

/* Another user struct with nesting */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct base;
  struct SimpleStruct* GTY((skip)) nested;
} ComplexUserStruct;

/* TYPE_UNION */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char* GTY((skip)) string_val;
};

/* TYPE_POINTER variations */
int* GTY(()) int_ptr;
struct SimpleStruct* GTY(()) struct_ptr;
UserStruct* GTY(()) user_struct_ptr;
union DataUnion* GTY(()) union_ptr;

/* TYPE_ARRAY variations */
int GTY(()) int_array[10];
float GTY(()) float_array[];
struct SimpleStruct GTY(()) struct_array[5];
UserStruct* GTY(()) ptr_array[20];

/* TYPE_CALLBACK - function pointer types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(struct SimpleStruct*, UserStruct**);

/* Callback variable declarations */
SimpleCallback GTY(()) simple_cb;
ComplexCallback GTY(()) complex_cb;

/* Callback that returns pointer to array */
typedef int (*CallbackReturningArrayPtr)(void)[10];
CallbackReturningArrayPtr GTY(()) array_cb;

/* TYPE_LANG_STRUCT with GCC attributes */
struct GTY(()) LangStruct __attribute__((transaction_safe)) {
  int transaction_id;
  void* GTY((skip)) transaction_data;
};

/* Another language struct with vector attribute */
struct GTY(()) VectorStruct __attribute__((vector_size(16))) {
  int data[4];
};

/* Complex nested type combining multiple categories */
struct GTY(()) ContainerStruct {
  /* Scalar members */
  int count;
  float total;
  
  /* String member */
  const char* GTY((skip)) description;
  
  /* Struct member */
  struct SimpleStruct embedded;
  
  /* User struct member */
  UserStruct user_data;
  
  /* Union member */
  union DataUnion data_union;
  
  /* Pointer members */
  int* GTY((skip)) dynamic_array;
  struct ContainerStruct* GTY((skip)) next;
  
  /* Array member */
  UserStruct items[8];
  
  /* Callback member */
  SimpleCallback on_update;
  
  /* Language struct member */
  struct LangStruct lang_data;
};

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  struct ContainerStruct container;
  ComplexCallback handler;
  void* GTY((skip)) data;
};

/* Array of pointers to unions */
union MixedUnion* GTY(()) union_ptr_array[15];

/* Function pointer returning pointer to array */
typedef int (*FuncReturningArrayPtr)(void)[5];
FuncReturningArrayPtr GTY(()) advanced_cb;

/* Struct with array of function pointers */
struct GTY(()) CallbackRegistry {
  SimpleCallback callbacks[10];
  ComplexCallback complex_handlers[3];
};

/* Global instances to ensure types are instantiated */
struct SimpleStruct GTY(()) simple_instance = {1, 2.0};
UserStruct GTY(()) user_instance = {42, "test"};
union DataUnion GTY(()) union_instance;
struct ContainerStruct GTY(()) container_instance;
struct LangStruct GTY(()) lang_instance;
struct CallbackRegistry GTY(()) registry_instance;

/* Function using the types to prevent dead code elimination */
void GTY(()) gt_test_function(void) {
  /* Use scalar types */
  global_int = 42;
  global_float = 3.14f;
  global_double = 2.71828;
  global_char = 'A';
  
  /* Use string */
  const char* local_str = global_string;
  
  /* Use structs */
  simple_instance.x = 100;
  user_instance.id = 200;
  
  /* Use union */
  union_instance.int_val = 300;
  
  /* Use pointers */
  int_ptr = &global_int;
  struct_ptr = &simple_instance;
  
  /* Use arrays */
  int_array[0] = 1;
  
  /* Use callbacks */
  if (simple_cb) {
    simple_cb(42);
  }
  
  /* Use container */
  container_instance.count = 10;
  container_instance.on_update = simple_cb;
  
  /* Use language struct */
  lang_instance.transaction_id = 1;
}

/* Another function with more complex type usage */
void GTY(()) gt_complex_test(void) {
  /* Nested array access */
  container_instance.items[0].id = 1;
  
  /* Pointer to array */
  int (*array_ptr)[10] = &int_array;
  
  /* Complex callback usage */
  if (complex_cb) {
    UserStruct* user_ptr = &user_instance;
    complex_cb(&simple_instance, &user_ptr);
  }
  
  /* Union array access */
  if (union_ptr_array[0]) {
    union_ptr_array[0]->container.count = 5;
  }
  
  /* Registry usage */
  registry_instance.callbacks[0] = simple_cb;
}

/* Main function for standalone compilation */
int main(int argc, char** argv) {
  gt_test_function();
  gt_complex_test();
  return 0;
}

/* Additional undefined type references to ensure they're counted */
struct OpaqueStruct* GTY(()) opaque_struct_ptr;
union OpaqueUnion* GTY(()) opaque_union_ptr;

/* Template-like structure for C++ mode */
#ifdef __cplusplus
class GTY(()) TestClass {
  int value;
  char* GTY((skip)) name;
public:
  TestClass() : value(0), name(0) {}
};
#endif
