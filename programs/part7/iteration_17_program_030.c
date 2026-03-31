/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in gcc/ directory and built with coverage-enabled GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* Forward declarations for TYPE_UNDEFINED */
struct OpaqueStruct;
union OpaqueUnion;
typedef struct OpaqueStruct *OpaquePtr;

/* Basic scalar types - TYPE_SCALAR */
int global_int GTY((skip));
float global_float GTY((skip));
double global_double GTY((skip));
char global_char GTY((skip));

/* String type - TYPE_STRING */
const char *global_string GTY((skip)) = "test string";

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
  char * GTY((skip)) string_val;
};

/* Pointer types - TYPE_POINTER */
int *int_ptr GTY((skip));
struct SimpleStruct *struct_ptr GTY((skip));
UserStruct *user_struct_ptr GTY((skip));
union DataUnion *union_ptr GTY((skip));

/* Array types - TYPE_ARRAY */
int int_array[10] GTY((skip));
struct SimpleStruct struct_array[5] GTY(());
UserStruct user_struct_array[] GTY(()) = {{1, "test1"}, {2, "test2"}};

/* Complex nested struct with arrays and pointers */
struct GTY(()) ComplexStruct {
  int count;
  struct SimpleStruct * GTY((skip)) items;
  union DataUnion data;
  int scores[20];
  void (*callback)(int);  /* Function pointer member */
};

/* Callback types - TYPE_CALLBACK */
typedef void (*SimpleCallback)(int, char*);
typedef int (*ComplexCallback)(struct ComplexStruct*, UserStruct*);

/* Function pointer variables */
SimpleCallback cb1 GTY((skip));
ComplexCallback cb2 GTY((skip));

/* Array of function pointers */
void (*callbacks[5])(void) GTY((skip));

/* Language-specific struct with GCC attributes - TYPE_LANG_STRUCT */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void * GTY((skip)) ptr;
};

/* Another language struct with transaction attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  char * GTY((skip)) name;
};

/* Struct containing union containing struct */
struct GTY(()) Container {
  union {
    struct SimpleStruct s;
    UserStruct u;
  } data;
  int type;
};

/* Pointer to array */
typedef int (*ArrayPtr)[10];
ArrayPtr array_ptr GTY((skip));

/* Function returning pointer to struct */
struct SimpleStruct* get_simple_struct(void) {
  static struct SimpleStruct ss = {0, 0.0f};
  return &ss;
}

/* Function taking callback */
void register_callback(void (*cb)(int)) {
  if (cb) cb(42);
}

/* Union with struct and callback */
union GTY(()) MixedUnion {
  struct ComplexStruct cs;
  void (*action)(struct ComplexStruct*);
  char buffer[256];
};

/* Self-referential struct */
struct GTY(()) TreeNode {
  int value;
  struct TreeNode * GTY((skip)) left;
  struct TreeNode * GTY((skip)) right;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*FuncReturningArrayPtr)(void)[10];

/* Struct with all type variations */
struct GTY(()) TypeVariations {
  /* Scalar */
  int scalar;
  
  /* Pointer */
  char * GTY((skip)) pointer;
  
  /* Array */
  int array[5];
  
  /* Struct */
  struct SimpleStruct nested_struct;
  
  /* Union */
  union DataUnion nested_union;
  
  /* Callback */
  void (*nested_callback)(void);
  
  /* Pointer to array */
  int (*ptr_to_array)[5];
  
  /* Array of pointers */
  int *ptr_array[3];
  
  /* Nested anonymous struct */
  struct {
    int x;
    int y;
  } point;
};

/* Global instances to ensure types are used */
struct ComplexStruct global_complex GTY(());
union MixedUnion global_mixed GTY(());
struct TypeVariations global_variations GTY(());
struct LangStruct global_lang_struct GTY(());
struct TransactionStruct global_transaction_struct GTY(());

/* Function that uses all types to prevent dead code elimination */
void GTY((skip)) use_all_types(void) {
  /* Use scalar types */
  global_int = 42;
  global_float = 3.14f;
  
  /* Use string */
  const char *local_str = global_string;
  
  /* Use structs */
  struct SimpleStruct ss = {1, 2.0f};
  UserStruct us = {100, "test"};
  
  /* Use union */
  union DataUnion du;
  du.int_val = 100;
  
  /* Use pointers */
  int_ptr = &global_int;
  struct_ptr = &ss;
  
  /* Use arrays */
  int_array[0] = 1;
  struct_array[0].x = 10;
  
  /* Use callbacks */
  if (cb1) cb1(1, "test");
  
  /* Use complex types */
  global_complex.count = 5;
  global_mixed.action = 0;
  global_variations.scalar = 100;
  global_lang_struct.data = 200;
  global_transaction_struct.value = 300;
  
  /* Use function pointers */
  register_callback(0);
  
  /* Reference undefined types */
  extern struct OpaqueStruct *get_opaque(void);
  struct OpaqueStruct *opaque = get_opaque();
}

/* Main function for standalone compilation */
int main(void) {
  use_all_types();
  return 0;
}

/* Additional file to simulate multi-file compilation */
#ifdef HEADER_FILE
/* test-types.h */
struct HeaderStruct {
  int header_field;
  float another_field;
};

typedef union {
  int x;
  float y;
} HeaderUnion;

extern void header_function(struct HeaderStruct*);
#endif
