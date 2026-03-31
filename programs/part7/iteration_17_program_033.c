/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for TYPE_UNDEFINED */
struct OpaqueStruct;          /* Undefined struct */
union OpaqueUnion;            /* Undefined union */

/* TYPE_SCALAR examples */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

/* TYPE_STRING - char* used as string */
typedef const char* string_t;

/* Basic struct for TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int field1;
  float field2;
};

/* TYPE_USER_STRUCT - typedef struct */
typedef struct GTY(()) {
  int x;
  double y;
  char* GTY((skip)) name;  /* Skip this pointer for GC */
} UserStruct;

/* Another user struct with complex nesting */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct* GTY((tag("0"))) us_ptr;  /* Pointer to user struct */
  int data[5];                         /* Fixed array */
} ComplexUserStruct;

/* TYPE_UNION */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char* GTY((skip)) str_val;  /* String pointer */
};

/* TYPE_POINTER variations */
typedef int* int_ptr_t;
typedef UserStruct* user_struct_ptr_t;
typedef void (*func_ptr_t)(void);  /* Function pointer */

/* TYPE_ARRAY variations */
typedef int fixed_array_t[10];
typedef int incomplete_array_t[];
typedef UserStruct* ptr_array_t[5];

/* TYPE_CALLBACK - function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* GTY((skip)) data);
typedef UserStruct* (*factory_t)(int id);

/* Complex callback returning pointer to array */
typedef int* (*array_factory_t)(size_t size);

/* TYPE_LANG_STRUCT - GCC extension attributes */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void* GTY((skip)) opaque;
};

/* Another lang struct with transaction_safe attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char buffer[32];
};

/* Nested type examples */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  DataUnion* GTY((length("count"))) unions[10];  /* Array of pointers to unions */
  int count;
  callback_t notify;  /* Callback function pointer */
};

/* Union containing struct and callback pointer */
union GTY(()) MegaUnion {
  struct GTY(()) {
    int type;
    void* GTY((skip)) data;
  } tagged_data;
  
  binary_op_t operation;  /* Function pointer */
  
  struct {
    fixed_array_t numbers;
    int size;
  } array_wrapper;
};

/* Typedef for function pointer returning pointer to array */
typedef fixed_array_t* (*array_ptr_factory_t)(void);

/* Global variables to ensure types are used */
static scalar_int_t global_int GTY((skip)) = 42;
static string_t global_string GTY((skip)) = "test";
static UserStruct global_user_struct GTY((skip));
static DataUnion global_union GTY((skip));
static int_ptr_t global_int_ptr GTY((skip)) = &global_int;
static fixed_array_t global_array GTY((skip));
static binary_op_t global_callback GTY((skip)) = NULL;
static LangStruct global_lang_struct GTY((skip));

/* Function using the complex types */
void GTY((skip)) gt_test_function(void) {
  /* Create instances */
  SimpleStruct simple GTY((skip));
  ComplexUserStruct complex GTY((skip));
  ContainerStruct container GTY((skip));
  MegaUnion mega GTY((skip));
  
  /* Use scalar types */
  simple.field1 = 10;
  simple.field2 = 3.14f;
  
  /* Use user struct */
  complex.us_ptr = &global_user_struct;
  complex.data[0] = 100;
  
  /* Use union */
  global_union.int_val = 255;
  
  /* Use array */
  global_array[0] = 1;
  global_array[9] = 10;
  
  /* Use pointer */
  *global_int_ptr = 99;
  
  /* Setup container with callback */
  container.count = 3;
  container.notify = NULL;
  
  /* Use mega union */
  mega.tagged_data.type = 1;
  mega.operation = NULL;
  mega.array_wrapper.size = 5;
  
  /* Reference lang struct */
  global_lang_struct.data = 777;
}

/* Another function using function pointers */
static int GTY((skip)) add_numbers(int a, int b) {
  return a + b;
}

static UserStruct* GTY((skip)) create_user_struct(int id) {
  static UserStruct instance;
  instance.x = id;
  instance.y = 3.14159;
  return &instance;
}

/* Test callback usage */
void GTY((skip)) test_callbacks(void) {
  binary_op_t adder = add_numbers;
  factory_t factory = create_user_struct;
  array_factory_t array_factory = NULL;
  array_ptr_factory_t array_ptr_factory = NULL;
  
  /* Use the callbacks */
  int result = adder(5, 3);
  UserStruct* us = factory(1);
  
  /* Silence unused warnings in real build */
  (void)result;
  (void)us;
  (void)array_factory;
  (void)array_ptr_factory;
}

/* Main function to ensure everything is referenced */
int main(int argc, char** argv GTY((skip))) {
  gt_test_function();
  test_callbacks();
  
  /* Reference undefined types to keep them in type system */
  struct OpaqueStruct* opaque_ptr GTY((skip)) = NULL;
  union OpaqueUnion* opaque_union_ptr GTY((skip)) = NULL;
  
  (void)opaque_ptr;
  (void)opaque_union_ptr;
  (void)argc;
  (void)argv;
  
  return 0;
}

/* Additional undefined type declarations in same TU */
struct ForwardDeclared;  /* TYPE_UNDEFINED */
union AnotherOpaque;     /* TYPE_UNDEFINED */

/* Complex nested type definition */
struct GTY(()) UltimateType {
  /* Mix of all type kinds */
  int scalar;                           /* TYPE_SCALAR */
  const char* GTY((skip)) string;       /* TYPE_STRING */
  SimpleStruct direct;                  /* TYPE_STRUCT */
  UserStruct user;                      /* TYPE_USER_STRUCT */
  DataUnion data;                       /* TYPE_UNION */
  void* GTY((skip)) pointer;            /* TYPE_POINTER */
  int array[7];                         /* TYPE_ARRAY */
  callback_t handler;                   /* TYPE_CALLBACK */
  LangStruct lang;                      /* TYPE_LANG_STRUCT */
  
  /* Nested complex member */
  struct GTY(()) {
    int tag;
    union GTY(()) {
      int as_int;
      float as_float;
      UserStruct* GTY((skip)) as_ptr;
    } value;
  } nested;
};

/* Force gengtype to process all types by creating a root variable */
static UltimateType GTY((skip)) root_instance;
