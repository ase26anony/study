/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in gcc/ directory and built with coverage-enabled GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Scalar types - TYPE_SCALAR */
typedef int my_int;
typedef float my_float;
typedef double my_double;
typedef char my_char;
typedef bool my_bool;

/* String type - TYPE_STRING */
typedef const char *my_string;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int x;
  double y;
  char * GTY((skip)) name;
} UserStruct;

/* Another struct with complex members */
struct GTY(()) ComplexStruct {
  int * GTY((skip)) int_ptr;           /* Pointer type */
  UserStruct user;                     /* User struct type */
  struct BasicStruct basic;            /* Struct type */
};

/* Union type - TYPE_UNION */
union GTY(()) MyUnion {
  int as_int;
  float as_float;
  double as_double;
  struct BasicStruct as_struct;
};

/* Array types - TYPE_ARRAY */
typedef int IntArray[10];
typedef struct BasicStruct StructArray[5];
typedef int (*FuncPtrArray[3])(void);  /* Array of function pointers */

/* Pointer types - TYPE_POINTER */
typedef int *IntPtr;
typedef struct BasicStruct *StructPtr;
typedef UserStruct *UserStructPtr;
typedef union MyUnion *UnionPtr;
typedef void (*VoidFuncPtr)(void);

/* Callback/function types - TYPE_CALLBACK */
typedef int (*BinaryFunc)(int, int);
typedef void (*CallbackFunc)(void * GTY((skip)) data);
typedef struct BasicStruct *(*StructFactory)(void);

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void * GTY((skip)) ptr;
};

/* Transaction-safe struct (GCC extension) */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char buffer[256];
};

/* Nested complex type definitions */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  union MyUnion * GTY((skip)) union_ptrs[8];
  IntArray numbers;
  struct ComplexStruct complex;
};

/* Union containing struct and callback pointer */
union GTY(()) MixedUnion {
  struct ContainerStruct container;
  CallbackFunc callback;
  BinaryFunc binary_func;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*(*ComplexFuncPtr)(void))[10];

/* Function pointer with complex return type */
typedef struct ContainerStruct *(*ContainerFactory)(int size);

/* Global variables to ensure types are used */
struct BasicStruct GTY((tag("BASIC"))) global_basic = {1, 2.0f};
UserStruct GTY((tag("USER"))) global_user = {10, 3.14, NULL};
union MyUnion GTY((tag("UNION"))) global_union;
struct ContainerStruct GTY((tag("CONTAINER"))) global_container;
struct LangStruct GTY((tag("LANG"))) global_lang = {42, NULL};
struct TransactionStruct GTY((tag("TRANSACTION"))) global_transaction = {100, ""};

/* Function using string type - TYPE_STRING */
static const char * GTY((skip)) get_version(void) {
  return "gtype-test-1.0";
}

/* Callback function implementation */
static int add_numbers(int a, int b) {
  return a + b;
}

static void simple_callback(void * GTY((skip)) data) {
  /* Do nothing */
}

/* Function that uses all the complex types */
void GTY((user)) gt_test_function(void) {
  /* Use scalar types */
  my_int i = 42;
  my_float f = 3.14f;
  my_double d = 2.71828;
  my_char c = 'A';
  my_bool b = true;
  
  /* Use string type */
  my_string version = get_version();
  
  /* Use pointer types */
  IntPtr int_ptr = &i;
  StructPtr struct_ptr = &global_basic;
  UserStructPtr user_ptr = &global_user;
  UnionPtr union_ptr = &global_union;
  
  /* Use array types */
  IntArray arr = {0};
  StructArray sarr;
  
  /* Use callback types */
  BinaryFunc adder = add_numbers;
  CallbackFunc cb = simple_callback;
  int result = adder(10, 20);
  cb(NULL);
  
  /* Use union type */
  global_union.as_int = 100;
  
  /* Use complex nested types */
  global_container.numbers[0] = 1;
  global_container.union_ptrs[0] = &global_union;
  
  /* Use language-specific structs */
  global_lang.data = 99;
  global_transaction.value = 200;
  
  /* Prevent unused variable warnings */
  (void)f;
  (void)d;
  (void)c;
  (void)b;
  (void)version;
  (void)int_ptr;
  (void)struct_ptr;
  (void)user_ptr;
  (void)union_ptr;
  (void)arr;
  (void)sarr;
  (void)result;
}

/* Additional forward declarations to ensure TYPE_UNDEFINED */
struct AnotherOpaque;
typedef struct YetAnotherOpaque YetAnotherOpaque;

/* Enum type (also scalar) */
typedef enum {
  RED,
  GREEN,
  BLUE
} Color;

/* Struct with bitfield */
struct GTY(()) BitfieldStruct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
};

/* Variable-length array in struct (GCC extension) */
struct GTY(()) VLAStruct {
  int length;
  int data[];
};

/* Main function to make file compilable */
int main(int argc, char **argv) {
  gt_test_function();
  
  /* Use some types to prevent optimization */
  struct BitfieldStruct bfs = {1, 2, 3};
  Color col = RED;
  
  (void)argc;
  (void)argv;
  (void)bfs;
  (void)col;
  
  return 0;
}

/* Additional type definitions in different scopes */

/* Namespace-like struct */
struct GTY(()) NamespaceStruct {
  struct {
    int inner_data;
  } inner;
  
  union {
    int as_int;
    float as_float;
  } value;
};

/* Typedef for pointer to array of function pointers */
typedef int (*(*NestedFuncPtr)[5])(void);

/* Struct with all type categories as members */
struct GTY(()) UltimateTestStruct {
  /* Scalar */
  int scalar_member;
  
  /* String */
  const char * GTY((skip)) string_member;
  
  /* Struct */
  struct BasicStruct struct_member;
  
  /* User struct */
  UserStruct user_struct_member;
  
  /* Union */
  union MyUnion union_member;
  
  /* Pointer */
  void * GTY((skip)) pointer_member;
  
  /* Array */
  int array_member[5];
  
  /* Callback */
  CallbackFunc callback_member;
  
  /* Language struct */
  struct LangStruct lang_struct_member;
};

/* Global instance */
struct UltimateTestStruct GTY((tag("ULTIMATE"))) global_ultimate = {
  42,
  "test",
  {1, 2.0f},
  {10, 3.14, NULL},
  {.as_int = 100},
  NULL,
  {1, 2, 3, 4, 5},
  simple_callback,
  {99, NULL}
};
