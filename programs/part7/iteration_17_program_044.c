/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;           /* TYPE_UNDEFINED */
union OpaqueUnion;             /* TYPE_UNDEFINED */

/* Scalar types - TYPE_SCALAR */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef bool scalar_bool_t;

/* String type - TYPE_STRING */
typedef const char *string_t;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int data;
  char * GTY((skip)) name;
} UserStruct;

/* Another user struct with complex nesting */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct * GTY((tag("0"))) user;
  struct BasicStruct basic;
} ComplexUserStruct;

/* Union type - TYPE_UNION */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char * GTY((skip)) string_val;
  struct BasicStruct * GTY((tag("1"))) struct_ptr;
};

/* Pointer types - TYPE_POINTER */
typedef int *int_ptr_t;
typedef struct BasicStruct *struct_ptr_t;
typedef UserStruct **double_ptr_t;
typedef void (*func_ptr_t)(void);

/* Array types - TYPE_ARRAY */
typedef int int_array_10[10];
typedef float float_array_5[5];
typedef struct BasicStruct struct_array_3[3];
typedef int incomplete_array[];

/* Callback types - TYPE_CALLBACK */
typedef int (*binary_func_t)(int, int);
typedef void (*callback_t)(int, const char *);
typedef struct BasicStruct *(*struct_factory_t)(void);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((transaction_safe)) {
  int transaction_id;
  void * GTY((skip)) transaction_data;
};

/* Another language struct with different attribute */
struct GTY(()) PackedStruct __attribute__((packed)) {
  char a;
  int b;
  char c;
};

/* Complex nested type definitions */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  union DataUnion * GTY((length("count"))) items[10];
  int count;
  int_array_10 indices;
};

/* Union containing struct and callback pointer */
union GTY(()) MixedUnion {
  struct ContainerStruct container;
  binary_func_t func_ptr;
  void * GTY((skip)) user_data;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*array_factory_t)(void)[10];

/* Struct with all type categories */
struct GTY(()) MasterStruct {
  /* Scalar */
  int scalar_field;
  float float_field;
  
  /* String */
  const char * GTY((skip)) string_field;
  
  /* Struct */
  struct BasicStruct nested_struct;
  
  /* User struct */
  UserStruct user_struct_field;
  
  /* Union */
  union DataUnion data_union;
  
  /* Pointer */
  struct MasterStruct * GTY((tag("2"))) self_ptr;
  
  /* Array */
  int int_array[5];
  UserStruct user_struct_array[3];
  
  /* Callback */
  callback_t callback_field;
  
  /* Language struct */
  struct LangStruct * GTY((tag("3"))) lang_struct_ptr;
  
  /* Undefined type pointer */
  struct OpaqueStruct * GTY((skip)) opaque_ptr;
};

/* Function pointer type definitions */
typedef void (*complex_callback_t)(struct MasterStruct *, union MixedUnion *);

/* Global variables to ensure types are used */
struct BasicStruct GTY((root)) global_basic_struct = {0, 0.0f};
UserStruct GTY((root)) global_user_struct = {0, NULL};
union DataUnion GTY((root)) global_union;
struct ContainerStruct GTY((root)) global_container;
struct MasterStruct GTY((root)) global_master_struct;
struct LangStruct GTY((root)) global_lang_struct;

/* Function using the complex types */
void GTY((user)) gt_test_function(void)
{
  /* Use scalar types */
  scalar_int_t i = 42;
  scalar_float_t f = 3.14f;
  (void)i;
  (void)f;
  
  /* Use string type */
  string_t str = "Hello, gengtype!";
  (void)str;
  
  /* Use pointer types */
  int_ptr_t ip = &i;
  struct_ptr_t sp = &global_basic_struct;
  (void)ip;
  (void)sp;
  
  /* Use array types */
  int_array_10 arr = {0};
  arr[0] = 1;
  
  /* Use callback type */
  binary_func_t add = NULL;
  (void)add;
  
  /* Access struct fields */
  global_master_struct.scalar_field = 100;
  global_master_struct.callback_field = NULL;
  
  /* Use union */
  global_union.int_val = 42;
  
  /* Use container */
  global_container.count = 0;
  
  /* Use language struct */
  global_lang_struct.transaction_id = 1;
}

/* Another function with different type usage */
static void process_types(void)
{
  /* Create local instances */
  ComplexUserStruct local_user;
  local_user.user = &global_user_struct;
  local_user.basic = global_basic_struct;
  
  /* Use mixed union */
  MixedUnion mixed;
  mixed.func_ptr = NULL;
  
  /* Array of pointers */
  struct BasicStruct *ptr_array[5];
  ptr_array[0] = &global_basic_struct;
  
  /* Nested array access */
  global_master_struct.int_array[0] = 10;
  global_master_struct.user_struct_array[0].data = 20;
  
  /* Pointer to array */
  int (*array_ptr)[5] = &global_master_struct.int_array;
  (*array_ptr)[1] = 30;
  
  /* Function pointer array */
  callback_t callbacks[3];
  callbacks[0] = NULL;
  
  /* Complex pointer chain */
  struct MasterStruct **master_pp = &global_master_struct.self_ptr;
  (void)master_pp;
}

/* Main function to ensure everything is referenced */
int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;
  
  gt_test_function();
  process_types();
  
  /* Ensure undefined types are declared */
  extern struct OpaqueStruct *get_opaque(void);
  extern union OpaqueUnion *get_opaque_union(void);
  
  return 0;
}

/* Additional type definitions in same file */

/* Another struct type */
struct GTY(()) AdditionalStruct {
  int id;
  char name[50];
  struct AdditionalStruct * GTY((tag("4"))) next;
};

/* Typedef creating another user struct */
typedef struct GTY(()) {
  int x, y, z;
} Point3D;

/* Array of unions */
union DataUnion union_array[10];

/* Pointer to function returning pointer */
typedef struct AdditionalStruct *(*allocator_t)(size_t);

/* Const pointer types */
typedef const int *const_int_ptr_t;
typedef const struct BasicStruct *const_struct_ptr_t;

/* Volatile types */
typedef volatile int volatile_int_t;
typedef volatile struct AdditionalStruct *volatile_struct_ptr_t;

/* Complete the undefined types with actual definitions */
struct OpaqueStruct {
  int hidden_data;
  void * GTY((skip)) secret;
};

union OpaqueUnion {
  long long_data;
  double double_data;
  struct OpaqueStruct * GTY((skip)) struct_ptr;
};
