/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in gcc/ directory and built with coverage-enabled GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations (TYPE_UNDEFINED) */
struct GTY(()) OpaqueStruct;      /* Undefined struct */
union GTY(()) OpaqueUnion;        /* Undefined union */

/* Scalar types (TYPE_SCALAR) */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef bool GTY(()) scalar_bool_t;

/* String type (TYPE_STRING) */
typedef const char * GTY(()) string_type_t;

/* Basic struct (TYPE_STRUCT) */
struct GTY(()) SimpleStruct {
  int field1;
  float field2;
};

/* User struct via typedef (TYPE_USER_STRUCT) */
typedef struct GTY(()) {
  int x;
  double y;
  char z;
} UserStruct;

/* Union type (TYPE_UNION) */
union GTY(()) DataUnion {
  int as_int;
  float as_float;
  char * GTY((skip)) as_string;
  struct SimpleStruct * GTY((skip)) as_struct;
};

/* Pointer types (TYPE_POINTER) */
typedef int * GTY(()) int_ptr_t;
typedef struct SimpleStruct * GTY(()) struct_ptr_t;
typedef union DataUnion * GTY(()) union_ptr_t;
typedef void (* GTY(()) func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_10[10];
typedef struct SimpleStruct GTY(()) struct_array_5[5];
typedef char GTY(()) char_array_incomplete[];

/* Callback/function types (TYPE_CALLBACK) */
typedef void (* GTY(()) CallbackFunc)(int, const char*);
typedef int (* GTY(()) CompareFunc)(const void*, const void*);

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct GTY(()) LangStruct {
  int data;
} __attribute__((transaction_safe));

/* Complex nested types */

/* Struct containing array of pointers to unions */
struct GTY(()) ComplexStruct1 {
  union DataUnion * GTY((length("count"))) * GTY((skip)) union_ptrs;
  int count;
  int_array_10 fixed_array;
  char_array_incomplete flexible_array;
};

/* Union containing struct and callback pointer */
union GTY(()) ComplexUnion {
  struct SimpleStruct nested_struct;
  CallbackFunc callback;
  int_ptr_t int_pointer;
};

/* Typedef for function pointer returning pointer to array */
typedef int (* GTY(()) (* GTY(()) ComplexFuncPtr)(void))[10];

/* Even more complex: struct with multiple nested types */
struct GTY(()) MasterStruct {
  /* Scalar */
  scalar_int_t id;
  
  /* String */
  string_type_t name;
  
  /* Struct */
  struct SimpleStruct simple;
  
  /* User struct */
  UserStruct user;
  
  /* Union */
  union DataUnion data;
  
  /* Pointer */
  struct_ptr_t next;
  
  /* Array */
  struct_array_5 items;
  
  /* Callback */
  CompareFunc comparator;
  
  /* Language struct */
  struct LangStruct * GTY((skip)) lang_struct_ptr;
  
  /* Complex nested */
  struct ComplexStruct1 * GTY((skip)) complex_ptr;
  
  /* Function pointer array */
  CallbackFunc GTY((skip)) handlers[3];
};

/* Global variables to ensure types are used */
struct SimpleStruct GTY(()) global_simple_struct = {1, 2.0f};
UserStruct GTY(()) global_user_struct = {10, 3.14, 'A'};
union DataUnion GTY(()) global_union = {.as_int = 42};
int_array_10 GTY(()) global_array = {0,1,2,3,4,5,6,7,8,9};
struct MasterStruct * GTY((root)) global_master = NULL;

/* Function using the types */
void GTY((user)) init_types(void) {
  /* This function uses various types to ensure they're processed */
  static struct MasterStruct master = {
    .id = 100,
    .name = "Test",
    .simple = {2, 3.0f},
    .user = {20, 6.28, 'B'},
    .data = {.as_float = 1.5f},
    .next = &global_simple_struct,
    .items = {{1,1.0f}, {2,2.0f}, {3,3.0f}, {4,4.0f}, {5,5.0f}},
    .comparator = NULL,
    .lang_struct_ptr = NULL,
    .complex_ptr = NULL,
    .handers = {NULL, NULL, NULL}
  };
  
  /* Use string type */
  const char * GTY((skip)) str = "Hello, gengtype!";
  
  /* Use pointer types */
  int * GTY((skip)) ptr = &master.id;
  
  /* Use array type */
  char GTY((skip)) arr[] = "array";
  
  /* Prevent unused variable warnings */
  (void)str;
  (void)ptr;
  (void)arr;
}

/* Another function with different type usage */
void GTY((user)) use_callback_types(void) {
  /* Define a callback function */
  static void my_callback(int x, const char* s) {
    (void)x;
    (void)s;
  }
  
  /* Use function pointer */
  CallbackFunc func = my_callback;
  
  /* Complex function pointer usage */
  int (* GTY((skip)) array_ptr)[10] = &global_array;
  
  (void)func;
  (void)array_ptr;
}

/* Main function for standalone compilation */
#ifdef STANDALONE_TEST
int main(void) {
  init_types();
  use_callback_types();
  return 0;
}
#endif
