/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Prevent C++ name mangling for GCC internals */
#ifdef __cplusplus
extern "C" {
#endif

/* ========== TYPE_UNDEFINED ========== */
/* Forward declarations without definitions */
struct GTY(()) OpaqueStruct;
union GTY(()) OpaqueUnion;
typedef struct GTY(()) AnotherOpaque AnotherOpaque_t;

/* ========== TYPE_SCALAR ========== */
/* Basic scalar types */
int GTY(()) global_int;
float GTY(()) global_float;
double GTY(()) global_double;
char GTY(()) global_char;
bool GTY(()) global_bool;
size_t GTY(()) global_size_t;

/* ========== TYPE_STRING ========== */
/* String type - char* with string literal context */
const char* GTY(()) global_string = "test string";
char* GTY(()) dynamic_string GTY((length("strlen(global_string)")));

/* ========== TYPE_STRUCT ========== */
/* Plain struct types */
struct GTY(()) SimpleStruct {
  int x;
  float y;
};

/* Struct with pointer members */
struct GTY(()) StructWithPointers {
  int* GTY((skip)) int_ptr;
  struct SimpleStruct* GTY((skip)) struct_ptr;
  char* GTY((skip)) char_ptr;
};

/* ========== TYPE_USER_STRUCT ========== */
/* Typedef struct types */
typedef struct GTY(()) {
  int id;
  char name[32];
} UserStruct;

typedef struct GTY(()) TaggedStruct {
  long tag;
  void* GTY((skip)) data;
} *TaggedStructPtr;

/* Another user struct with complex nesting */
typedef struct GTY(()) ComplexUserStruct {
  UserStruct base;
  struct StructWithPointers* GTY((skip)) ptr_field;
  int array_field[10];
} ComplexUserStruct;

/* ========== TYPE_UNION ========== */
/* Union types */
union GTY(()) SimpleUnion {
  int as_int;
  float as_float;
  char* GTY((skip)) as_string;
};

/* Union containing a struct */
union GTY(()) UnionWithStruct {
  struct SimpleStruct as_struct;
  UserStruct as_user_struct;
  double as_double;
};

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* GTY((skip)) global_int_ptr;
struct SimpleStruct** GTY((skip)) global_struct_ptr_ptr;
UserStruct* GTY((skip)) global_user_struct_ptr;
union SimpleUnion* GTY((skip)) global_union_ptr;

/* Pointer to array */
typedef int (*GTY((skip)) ArrayPtr)[10];

/* ========== TYPE_ARRAY ========== */
/* Array types */
int GTY(()) global_int_array[100];
struct SimpleStruct GTY(()) struct_array[50];
UserStruct GTY(()) user_struct_array[25];

/* Multi-dimensional array */
double GTY(()) global_matrix[10][20];

/* Array of pointers */
int* GTY((skip)) pointer_array[30];

/* Incomplete array in struct */
struct GTY(()) StructWithFlexArray {
  int count;
  double data[];  /* Flexible array member */
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*GTY((skip)) SimpleCallback)(int, char*);
typedef void (*GTY((skip)) ComplexCallback)(struct SimpleStruct*, UserStruct*);

/* Struct with callback member */
struct GTY(()) StructWithCallback {
  int id;
  SimpleCallback GTY((skip)) callback;
  ComplexCallback GTY((skip)) complex_cb;
};

/* Union with callback */
union GTY(()) UnionWithCallback {
  SimpleCallback GTY((skip)) func_ptr;
  int* GTY((skip)) data_ptr;
};

/* Callback returning pointer to array */
typedef int (*GTY((skip)) CallbackReturningArrayPtr)(void)[5];

/* ========== TYPE_LANG_STRUCT ========== */
/* Structs with GCC attributes that might create TYPE_LANG_STRUCT */
struct GTY(()) LangStruct1 __attribute__((aligned(16))) {
  int x __attribute__((mode(SI)));
  long y __attribute__((mode(DI)));
};

struct GTY(()) LangStruct2 __attribute__((packed)) {
  char a;
  int b;
  double c;
};

/* Transaction-safe struct (GCC extension) */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  void* GTY((skip)) ptr;
};

/* Vector type struct */
typedef int v4si __attribute__((vector_size(16)));
struct GTY(()) VectorStruct {
  v4si vectors[4];
  int count;
};

/* ========== COMPLEX NESTING EXAMPLES ========== */
/* Struct containing array of pointers to unions */
struct GTY(()) SuperNestedStruct {
  int magic;
  union UnionWithCallback* GTY((skip)) union_ptrs[20];
  struct LangStruct1 lang_struct;
  CallbackReturningArrayPtr GTY((skip)) nested_callback;
};

/* Union containing struct and callback pointer */
union GTY(()) MegaUnion {
  struct SuperNestedStruct nested;
  ComplexCallback GTY((skip)) mega_callback;
  TaggedStructPtr tagged;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*(*GTY((skip)) UltraComplexType)(void))[10];

/* Array of structs containing unions */
struct GTY(()) ContainerStruct {
  union MegaUnion items[5];
  UltraComplexType GTY((skip)) processor;
};

/* ========== DUMMY FUNCTION TO REFERENCE TYPES ========== */
void GTY((skip)) gt_test_function(void) {
  /* Reference global variables to prevent elimination */
  global_int = 42;
  global_float = 3.14f;
  
  /* Use string type */
  if (global_string) {
    global_char = global_string[0];
  }
  
  /* Create instances (in reality, gengtype only sees declarations) */
  struct SimpleStruct local_struct = {1, 2.0f};
  UserStruct local_user_struct = {100, "test"};
  union SimpleUnion local_union;
  local_union.as_int = 255;
  
  /* Reference arrays */
  global_int_array[0] = 1;
  struct_array[0].x = 2;
  user_struct_array[0].id = 3;
  
  /* Reference pointers */
  if (global_int_ptr) {
    *global_int_ptr = 99;
  }
  
  /* Reference lang structs */
  struct LangStruct1 ls1 = {0};
  ls1.x = 10;
  
  struct TransactionStruct ts = {0};
  ts.value = 100;
}

/* ========== MAIN FUNCTION FOR COMPILATION VALIDITY ========== */
#ifdef TEST_STANDALONE
int main() {
  gt_test_function();
  return 0;
}
#endif

#ifdef __cplusplus
}
#endif
