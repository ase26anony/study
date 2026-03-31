/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in the gcc/ directory of the GCC source tree */

/* GCC boilerplate headers */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Prevent C++ name mangling for C compatibility */
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
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef _Bool GTY(()) scalar_bool_t;
typedef enum { RED, GREEN, BLUE } GTY(()) ColorEnum;

/* ========== TYPE_STRING ========== */
/* String types - char* with string context */
typedef const char* GTY(()) string_type_t;
typedef char* GTY(()) mutable_string_t;

/* ========== TYPE_STRUCT ========== */
/* Plain struct types */
struct GTY(()) SimpleStruct {
  int a;
  float b;
  char c;
};

struct GTY(()) NestedStruct {
  struct SimpleStruct inner;
  double extra;
};

/* ========== TYPE_USER_STRUCT ========== */
/* Typedef'd struct types */
typedef struct GTY(()) {
  int x;
  int y;
} Point;

typedef struct GTY(()) {
  Point start;
  Point end;
  ColorEnum color;
} LineSegment;

/* Complex user struct with various members */
typedef struct GTY(()) ComplexUserStruct {
  scalar_int_t id;
  scalar_float_t weight;
  string_type_t name;
  struct SimpleStruct* GTY((skip)) simple_ptr;
  Point position;
  int GTY((length("dim"))) *array_ptr;
  int dim;
} ComplexUserStruct_t;

/* ========== TYPE_UNION ========== */
/* Union types */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char* GTY((skip)) string_val;
  struct SimpleStruct struct_val;
};

typedef union GTY(()) TaggedUnion {
  int type;
  struct {
    int x;
    int y;
  } point;
  struct {
    float radius;
    ColorEnum color;
  } circle;
} TaggedUnion_t;

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
typedef int* GTY((skip)) int_ptr_t;
typedef struct SimpleStruct* GTY((skip)) struct_ptr_t;
typedef union DataUnion* GTY((skip)) union_ptr_t;
typedef ComplexUserStruct_t* GTY((skip)) user_struct_ptr_t;
typedef void (*GTY((skip)) void_func_ptr_t)(void);

/* Pointer to pointer */
typedef int** GTY((skip)) int_ptr_ptr_t;

/* ========== TYPE_ARRAY ========== */
/* Array types */
typedef int GTY(()) int_array_10_t[10];
typedef float GTY(()) float_array_5_t[5];
typedef struct SimpleStruct GTY(()) struct_array_3_t[3];
typedef Point GTY(()) point_array_dyn_t[];  /* Incomplete array */

/* Array of pointers */
typedef int* GTY((skip)) ptr_array_8_t[8];

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types (callbacks) */
typedef int (*GTY((skip)) CompareFunc)(const void*, const void*);
typedef void (*GTY((skip)) VoidCallback)(void* data, int status);
typedef ComplexUserStruct_t* (*GTY((skip)) CreateFunc)(int id, const char* name);

/* Struct with callback member */
struct GTY(()) CallbackContainer {
  int id;
  VoidCallback GTY((skip)) callback;
  void* GTY((skip)) user_data;
};

/* Union with callback */
union GTY(()) CallbackUnion {
  VoidCallback GTY((skip)) void_cb;
  CompareFunc GTY((skip)) compare_cb;
  int regular_int;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Structs with GCC attributes that might create TYPE_LANG_STRUCT */
struct GTY(()) TransactionSafeStruct 
  __attribute__((transaction_safe)) {
  int transaction_id;
  double amount;
  char description[100];
};

struct GTY(()) AlignedStruct 
  __attribute__((aligned(64))) {
  int counter;
  double values[8];
};

/* Struct with mode attribute */
struct GTY(()) ModeStruct 
  __attribute__((mode(SI))) {
  int data;
};

/* ========== COMPLEX NESTED STRUCTURES ========== */
/* Richly nested type to ensure deep traversal */

typedef struct GTY(()) TreeNode {
  int value;
  struct TreeNode* GTY((skip)) left;
  struct TreeNode* GTY((skip)) right;
  VoidCallback GTY((skip)) visit_cb;
} TreeNode_t;

struct GTY(()) GraphNode {
  int id;
  string_type_t label;
  struct GraphNode** GTY((skip)) neighbors;  /* Array of pointers */
  int neighbor_count;
  union DataUnion node_data;
};

typedef struct GTY(()) Container {
  /* Mix of all type kinds */
  scalar_int_t scalar_field;
  string_type_t string_field;
  struct SimpleStruct struct_field;
  Point user_struct_field;
  union DataUnion union_field;
  int* GTY((skip)) pointer_field;
  int_array_10_t array_field;
  VoidCallback GTY((skip)) callback_field;
  struct TransactionSafeStruct lang_struct_field;
  struct OpaqueStruct* GTY((skip)) undefined_ptr_field;
  
  /* Nested arrays and pointers */
  struct GraphNode* GTY((skip)) graph_nodes[5];
  CompareFunc GTY((skip)) compare_funcs[3];
  
  /* Flexible array member */
  TaggedUnion_t flexible_array[];
} Container_t;

/* ========== FUNCTION DECLARATIONS ========== */
/* Functions that use the types to prevent dead code elimination */

void GTY((skip)) init_types(void) {
  /* This function is just to reference types, not actually called */
  static int dummy = 0;
  dummy++;
}

ComplexUserStruct_t* GTY((skip)) create_complex_struct(int id, const char* name) {
  /* Dummy function for callback type */
  return NULL;
}

int GTY((skip)) compare_ints(const void* a, const void* b) {
  return 0;
}

/* ========== GLOBAL VARIABLES ========== */
/* Global instances to ensure types are instantiated */

struct SimpleStruct GTY(()) global_simple_struct = {1, 2.0f, 'A'};
Point GTY(()) global_point = {10, 20};
union DataUnion GTY(()) global_union = {.int_val = 42};
Container_t* GTY((skip)) global_container = NULL;
string_type_t GTY(()) global_string = "Hello, gengtype!";
int_array_10_t GTY(()) global_array = {0,1,2,3,4,5,6,7,8,9};
struct TransactionSafeStruct GTY(()) global_lang_struct = {100, 99.99, "Transaction"};

/* Array of various types */
void* GTY((skip)) global_void_ptr_array[] = {
  &global_simple_struct,
  &global_point,
  &global_union,
  (void*)global_string,
  &global_array[0]
};

/* ========== MAIN TEST FUNCTION ========== */
/* This would be called if the file were compiled normally */

#ifdef TEST_COMPILE
int main() {
  /* Create instances to use the types */
  ComplexUserStruct_t local_complex = {
    .id = 1,
    .weight = 75.5f,
    .name = "Test",
    .simple_ptr = &global_simple_struct,
    .position = {.x = 100, .y = 200},
    .array_ptr = global_array,
    .dim = 10
  };
  
  TreeNode_t tree_node = {
    .value = 42,
    .left = NULL,
    .right = NULL,
    .visit_cb = NULL
  };
  
  /* Use string type */
  const char* message = global_string;
  
  /* Use callback type */
  CompareFunc comparator = compare_ints;
  
  return 0;
}
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif
