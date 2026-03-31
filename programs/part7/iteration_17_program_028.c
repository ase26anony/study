/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Scalar types - TYPE_SCALAR */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef _Bool scalar_bool_t;

/* String type - TYPE_STRING */
typedef const char *string_ptr_t;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
} UserStruct;

/* Another user struct with more complexity */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct *GTY((skip)) us_ptr;
  int data[5];
} ComplexUserStruct;

/* Union type - TYPE_UNION */
union GTY(()) BasicUnion {
  int as_int;
  float as_float;
  char *GTY((skip)) as_string;
  struct BasicStruct *GTY((skip)) as_struct;
};

/* Pointer types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct BasicStruct *struct_ptr_t;
typedef union BasicUnion *union_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array types - TYPE_ARRAY */
typedef int int_array_10_t[10];
typedef float float_array_t[];
typedef struct BasicStruct struct_array_t[5];
typedef int *pointer_array_t[8];

/* Complex nested array type */
typedef struct GTY(()) ArrayContainer {
  int multi_dim[3][4][5];
  char *GTY((skip)) string_array[10];
} ArrayContainer;

/* Callback/function types - TYPE_CALLBACK */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct BasicStruct *, union BasicUnion *);
typedef char *(*string_callback_t)(const char *);

/* Struct containing callbacks */
struct GTY(()) CallbackContainer {
  simple_callback_t func1;
  complex_callback_t func2;
  string_callback_t func3;
};

/* Language-specific struct - TYPE_LANG_STRUCT */
/* Using GCC attributes to potentially create language-specific types */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void *GTY((skip)) ptr;
};

/* Another language struct with transaction_safe attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char *GTY((skip)) name;
};

/* Complex nested type combining multiple categories */
struct GTY(()) SuperComplexType {
  /* Scalar */
  int id;
  
  /* String */
  const char *GTY((skip)) description;
  
  /* Struct pointer */
  struct BasicStruct *GTY((skip)) base_struct;
  
  /* User struct */
  UserStruct user_data;
  
  /* Union */
  union BasicUnion variant;
  
  /* Array of pointers */
  void *GTY((skip)) ptr_array[8];
  
  /* Callback */
  simple_callback_t processor;
  
  /* Nested array within union */
  union {
    int ints[10];
    float floats[10];
  } data_union;
  
  /* Pointer to array */
  int (*matrix_ptr)[4][4];
};

/* Function pointer returning pointer to array */
typedef int (*func_returning_array_ptr_t)(void)[10];

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  struct {
    int type;
    void *GTY((skip)) data;
  } tagged;
  void (*action)(void);
  int (*compute)(int, int);
};

/* Struct with flexible array member */
struct GTY(()) FlexStruct {
  int count;
  double values[];
};

/* Typedef for complex function pointer type */
typedef struct SuperComplexType *(*factory_func_t)(
  int, 
  const char *GTY((skip)), 
  simple_callback_t
);

/* Global variables to ensure types are instantiated */
struct BasicStruct GTY((root)) global_struct = {1, 2.0f};
UserStruct GTY((root)) global_user_struct = {10, 3.14};
union BasicUnion GTY((root)) global_union;
ArrayContainer GTY((root)) global_array_container;
struct LangStruct GTY((root)) global_lang_struct = {42, NULL};
struct TransactionStruct GTY((root)) global_transaction_struct = {100, "test"};

/* Dummy function using all types to prevent dead code elimination */
void GTY((root)) gt_test_function(void)
{
  /* Use scalar types */
  scalar_int_t i = 42;
  scalar_float_t f = 3.14f;
  (void)i;
  (void)f;
  
  /* Use string type */
  string_ptr_t str = "Hello, gengtype!";
  (void)str;
  
  /* Use pointer types */
  int_ptr_t ip = &i;
  struct_ptr_t sp = &global_struct;
  (void)ip;
  (void)sp;
  
  /* Use array types */
  int_array_10_t arr = {0,1,2,3,4,5,6,7,8,9};
  (void)arr;
  
  /* Use callback types */
  simple_callback_t cb = NULL;
  (void)cb;
  
  /* Access language structs */
  global_lang_struct.data = 99;
  global_transaction_struct.value = 200;
  
  /* Use the complex type */
  struct SuperComplexType complex_instance;
  complex_instance.id = 1;
  complex_instance.processor = NULL;
  
  /* Use mixed union */
  union MixedUnion mu;
  mu.tagged.type = 0;
  
  /* Reference undefined types (forward declarations) */
  struct OpaqueStruct *opaque_ptr = NULL;
  union OpaqueUnion *opaque_union_ptr = NULL;
  (void)opaque_ptr;
  (void)opaque_union_ptr;
}

/* Additional function to test function pointer arrays */
void GTY((root)) test_function_pointers(void)
{
  /* Array of function pointers */
  void (*func_array[5])(void);
  
  /* Pointer to array of function pointers */
  void (*(*complex_func_ptr)[5])(void);
  
  /* Function returning pointer to function */
  void (*(*get_callback(void))(void))(void) {
    return NULL;
  }
  
  (void)func_array;
  (void)complex_func_ptr;
  (void)get_callback;
}

/* Main function for standalone compilation test */
int main(int argc, char **argv)
{
  gt_test_function();
  test_function_pointers();
  return 0;
}
