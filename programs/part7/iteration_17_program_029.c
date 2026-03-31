/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef bool GTY(()) scalar_bool_t;

/* String type - TYPE_STRING */
typedef const char * GTY(()) string_type_t;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct (typedef struct) - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  int y;
} UserStruct;

/* Another user struct with complex members */
typedef struct GTY(()) {
  UserStruct* GTY((skip)) us_ptr;
  int data;
} ComplexUserStruct;

/* Union type - TYPE_UNION */
union GTY(()) BasicUnion {
  int as_int;
  float as_float;
  char* GTY((skip)) as_string;
};

/* Pointer types - TYPE_POINTER */
typedef int* GTY((skip)) int_ptr_t;
typedef BasicStruct* GTY((skip)) struct_ptr_t;
typedef void (*GTY((skip)) func_ptr_t)(void);

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_10[10];
typedef BasicStruct GTY(()) struct_array_5[5];
extern int GTY(()) incomplete_array[];

/* Complex nested array of pointers */
typedef UserStruct* GTY((skip)) ptr_array[8];

/* Callback/function types - TYPE_CALLBACK */
typedef int (*GTY((skip)) callback_t)(int, float);
typedef void (*GTY((skip)) complex_callback_t)(UserStruct*, int_array_10);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((transaction_safe)) {
  int safe_field;
  void* GTY((skip)) safe_ptr;
};

/* Another language struct with GCC attribute */
struct GTY(()) PackedStruct __attribute__((packed)) {
  char a;
  int b;
  char c;
};

/* Complex nested type combining multiple categories */
struct GTY(()) ContainerStruct {
  /* Scalar */
  int id;
  
  /* String */
  const char* GTY((skip)) name;
  
  /* Struct */
  BasicStruct basic;
  
  /* User struct */
  UserStruct user;
  
  /* Union */
  BasicUnion data_union;
  
  /* Pointer */
  ContainerStruct* GTY((skip)) next;
  
  /* Array */
  int scores[5];
  
  /* Array of pointers */
  UserStruct* GTY((skip)) users[3];
  
  /* Callback */
  callback_t GTY((skip)) processor;
  
  /* Language struct */
  LangStruct lang_data;
};

/* Union containing multiple complex types */
union GTY(()) MegaUnion {
  ContainerStruct as_container;
  LangStruct as_lang;
  callback_t GTY((skip)) as_callback;
  int_array_10 as_array;
};

/* Function pointer returning pointer to array */
typedef int (*GTY((skip)) (*complex_func_ptr_t)(void))[10];

/* Another complex callback type */
typedef union GTY(()) {
  int (*GTY((skip)) func_int)(int);
  float (*GTY((skip)) func_float)(float);
} callback_union_t;

/* Struct with forward declared member */
struct GTY(()) HasForwardRef {
  struct OpaqueStruct* GTY((skip)) opaque_ptr;
  union OpaqueUnion* GTY((skip)) opaque_union_ptr;
};

/* Now define the previously forward-declared types */
struct GTY(()) OpaqueStruct {
  int defined_now;
  ContainerStruct* GTY((skip)) container;
};

union GTY(()) OpaqueUnion {
  int value;
  OpaqueStruct* GTY((skip)) opaque_ptr;
};

/* Global variables to ensure types are instantiated */
BasicStruct GTY(()) global_struct = {1, 2.0f};
UserStruct GTY(()) global_user_struct = {10, 20};
BasicUnion GTY(()) global_union = {.as_int = 42};
ContainerStruct GTY(()) global_container = {
  .id = 100,
  .name = "test",
  .basic = {1, 2.0f},
  .user = {10, 20},
  .data_union = {.as_int = 42},
  .next = 0,
  .scores = {1, 2, 3, 4, 5},
  .users = {0, 0, 0},
  .processor = 0,
  .lang_data = {5, 0}
};
LangStruct GTY(()) global_lang_struct = {7, 0};
int GTY(()) global_array[5] = {1, 2, 3, 4, 5};

/* Function using the complex types */
void GTY((skip)) process_types(void) {
  /* Create local instances */
  UserStruct local_user = {30, 40};
  BasicUnion local_union;
  local_union.as_int = 99;
  
  /* Use array type */
  int_array_10 local_array = {0};
  
  /* Use callback */
  callback_t my_callback = 0;
  
  /* Use pointer types */
  ContainerStruct* GTY((skip)) container_ptr = &global_container;
  int_ptr_t int_ptr = &global_container.id;
  
  /* Reference language struct */
  LangStruct* GTY((skip)) lang_ptr = &global_lang_struct;
  
  /* Use the mega union */
  MegaUnion mega;
  mega.as_container = global_container;
}

/* Another function with different signature */
int GTY((skip)) typed_callback(int a, float b) {
  return a + (int)b;
}

/* Main function to ensure everything is referenced */
int main(int argc, char** argv) {
  process_types();
  
  /* Force references to all global types */
  global_struct.field1++;
  global_user_struct.x++;
  global_union.as_int++;
  global_container.id++;
  global_lang_struct.safe_field++;
  global_array[0]++;
  
  /* Use typed callback */
  callback_t cb = typed_callback;
  if (cb) cb(1, 2.0f);
  
  return 0;
}
