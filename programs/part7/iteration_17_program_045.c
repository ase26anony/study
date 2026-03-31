/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Forward declarations to create TYPE_UNDEFINED */
struct GTY(()) opaque_struct;      /* TYPE_UNDEFINED */
union GTY(()) opaque_union;        /* TYPE_UNDEFINED */

/* Basic scalar types - TYPE_SCALAR */
int GTY(()) global_int = 42;
float GTY(()) global_float = 3.14f;
double GTY(()) global_double = 2.71828;
char GTY(()) global_char = 'A';

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

/* Union type - TYPE_UNION */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char* GTY((skip)) string_val;
};

/* Pointer types - TYPE_POINTER */
int* GTY(()) int_ptr = NULL;
struct SimpleStruct* GTY(()) struct_ptr = NULL;
UserStruct* GTY(()) user_struct_ptr = NULL;

/* Array types - TYPE_ARRAY */
int GTY(()) int_array[10] = {0};
float GTY(()) float_array[] = {1.0f, 2.0f, 3.0f};
struct SimpleStruct GTY(()) struct_array[5];

/* Callback/function pointer - TYPE_CALLBACK */
typedef void (*CallbackFunc)(int, const char*);
CallbackFunc GTY(()) my_callback = NULL;

/* More complex callback signature */
typedef int (*ComplexCallback)(struct SimpleStruct*, UserStruct**, int[10]);

/* Language-specific struct with attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void* GTY((skip)) opaque;
};

/* Complex nested types to ensure thorough traversal */

/* Struct containing array of pointers */
struct GTY(()) ContainerStruct {
  int count;
  struct SimpleStruct* GTY((length("count"))) items[];
};

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  struct SimpleStruct simple;
  UserStruct user;
  CallbackFunc callback;
  int* GTY((skip)) int_ptr_field;
};

/* Struct with function pointer member */
struct GTY(()) CallbackContainer {
  const char* GTY((skip)) name;
  CallbackFunc handler;
  ComplexCallback complex_handler;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*FuncReturningArrayPtr)(void)[10];

/* Even more complex: struct with union of arrays */
struct GTY(()) SuperComplex {
  enum { INT_ARRAY, FLOAT_ARRAY, STRUCT_ARRAY } type;
  union {
    int GTY((tag("type == INT_ARRAY"))) ints[20];
    float GTY((tag("type == FLOAT_ARRAY"))) floats[20];
    struct SimpleStruct GTY((tag("type == STRUCT_ARRAY"))) structs[10];
  } data;
};

/* Global instances to ensure they're processed */
struct SimpleStruct GTY(()) global_simple = {1, 2.0f};
UserStruct GTY(()) global_user = {100, "test"};
union DataUnion GTY(()) global_union = {.int_val = 42};
struct LangStruct GTY(()) global_lang = {99, NULL};
struct ContainerStruct* GTY(()) global_container = NULL;
union MixedUnion GTY(()) global_mixed;
struct CallbackContainer GTY(()) global_callback_container = {
  "test", NULL, NULL
};
struct SuperComplex GTY(()) global_super = {INT_ARRAY, {.ints = {0}}};

/* Function definitions using the types */
static void
test_callback(int x, const char* s)
{
  /* Dummy function body */
  (void)x;
  (void)s;
}

static int
test_complex_callback(struct SimpleStruct* s, UserStruct** u, int arr[10])
{
  if (s && u && arr) return 1;
  return 0;
}

/* Dummy function that references all types to prevent elimination */
void
gt_test_function(void)
{
  /* Reference scalars */
  global_int++;
  global_float += 1.0f;
  global_double *= 2.0;
  global_char = 'B';
  
  /* Reference string */
  const char* local_str = global_string;
  (void)local_str;
  
  /* Reference structs */
  global_simple.x = 10;
  global_user.id = 200;
  
  /* Reference union */
  global_union.float_val = 3.14f;
  
  /* Reference pointers */
  if (int_ptr) *int_ptr = 5;
  if (struct_ptr) struct_ptr->x = 1;
  
  /* Reference arrays */
  int_array[0] = 1;
  float_array[1] = 2.5f;
  struct_array[0].x = 3;
  
  /* Reference callbacks */
  my_callback = test_callback;
  if (my_callback) my_callback(1, "test");
  
  /* Reference language struct */
  global_lang.data = 100;
  
  /* Reference container */
  if (global_container) {
    global_container->count = 0;
  }
  
  /* Reference mixed union */
  global_mixed.simple.x = 5;
  
  /* Reference callback container */
  global_callback_container.complex_handler = test_complex_callback;
  
  /* Reference super complex */
  global_super.data.ints[0] = 99;
}

/* Main function for standalone compilation test */
int
main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  
  gt_test_function();
  
  return 0;
}
