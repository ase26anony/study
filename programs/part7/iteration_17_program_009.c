/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in gcc/ directory and added to gcc_objs */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Prevent warnings about unused functions/variables */
#define USED __attribute__((used))

/* ========== TYPE_UNDEFINED ========== */
/* Forward declarations without definitions */
struct GTY(()) OpaqueStruct;
union GTY(()) OpaqueUnion;
class GTY(()) OpaqueClass;

/* ========== TYPE_SCALAR ========== */
/* Basic scalar types */
typedef int GTY(()) scalar_int;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;
typedef bool GTY(()) scalar_bool;
typedef long GTY(()) scalar_long;
typedef unsigned int GTY(()) scalar_unsigned;

/* ========== TYPE_STRING ========== */
/* String types - char* with string context */
typedef const char* GTY(()) string_type;
static const char* GTY((skip)) global_string = "test string literal";

/* ========== TYPE_STRUCT ========== */
/* Plain struct types */
struct GTY(()) SimpleStruct {
  int field1;
  float field2;
};

struct GTY(()) NestedStruct {
  SimpleStruct inner;
  double extra;
};

/* ========== TYPE_USER_STRUCT ========== */
/* Typedef struct types */
typedef struct GTY(()) {
  int x;
  int y;
} Point;

typedef struct GTY(()) {
  Point start;
  Point end;
  const char* GTY((skip)) name;
} LineSegment;

/* Another user struct with function pointer */
typedef struct GTY(()) {
  int (*GTY((skip)) compare)(int, int);
  void* GTY((skip)) user_data;
} Comparator;

/* ========== TYPE_UNION ========== */
/* Union types */
union GTY(()) DataUnion {
  int as_int;
  float as_float;
  double as_double;
  void* GTY((skip)) as_pointer;
};

union GTY(()) TaggedUnion {
  struct GTY(()) {
    int type;
    union {
      int ival;
      float fval;
    } GTY((skip)) value;
  } tagged;
  char raw_data[16];
};

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
typedef int* GTY((skip)) int_ptr;
typedef SimpleStruct* GTY((skip)) struct_ptr;
typedef DataUnion* GTY((skip)) union_ptr;
typedef void (*GTY((skip)) void_func_ptr)(void);
typedef int (*GTY((skip)) int_func_ptr)(int, int);

/* Pointer to pointer */
typedef int** GTY((skip)) int_ptr_ptr;

/* Pointer in struct */
struct GTY(()) PointerHolder {
  int* GTY((skip)) int_pointer;
  SimpleStruct* GTY((skip)) struct_pointer;
  void (*GTY((skip)) callback)(void);
};

/* ========== TYPE_ARRAY ========== */
/* Array types */
typedef int GTY(()) int_array[10];
typedef SimpleStruct GTY(()) struct_array[5];
typedef Point* GTY((skip)) pointer_array[8];

/* Incomplete array in struct */
struct GTY(()) FlexibleArray {
  int length;
  int data GTY((length("%0.length"))) [];
};

/* Multi-dimensional array */
typedef int GTY(()) matrix[3][3];

/* Array of pointers */
struct GTY(()) ArrayOfPointers {
  int count;
  void* GTY((skip)) items[4];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types (callbacks) */
typedef int (*GTY((skip)) CompareFunc)(const void*, const void*);
typedef void (*GTY((skip)) VoidCallback)(void* GTY((skip)) data);
typedef struct GTY(())* (*GTY((skip)) AllocatorFunc)(size_t);

/* Struct with callback */
struct GTY(()) EventHandler {
  const char* GTY((skip)) event_name;
  VoidCallback GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Union with callback */
union GTY(()) CallbackUnion {
  CompareFunc GTY((skip)) compare;
  VoidCallback GTY((skip)) simple;
  struct GTY(()) {
    AllocatorFunc GTY((skip)) alloc;
    void (*GTY((skip)) free)(void*);
  } memory_ops;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs using GCC attributes */
struct GTY(()) TransactionSafeStruct 
  __attribute__((transaction_safe)) {
  int value;
  void* GTY((skip)) data;
};

struct GTY(()) AlignedStruct 
  __attribute__((aligned(64))) {
  double coordinates[3];
  int id;
};

/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) VectorStruct {
  v4si vectors[2];
  int count;
};

/* ========== COMPLEX NESTED TYPES ========== */
/* Deeply nested type to ensure thorough traversal */

struct GTY(()) ComplexType {
  /* Scalar */
  int id;
  
  /* String */
  const char* GTY((skip)) name;
  
  /* Struct */
  SimpleStruct basic;
  
  /* User struct */
  Point position;
  
  /* Union */
  DataUnion variant;
  
  /* Pointer */
  ComplexType* GTY((skip)) next;
  
  /* Array */
  int values[5];
  
  /* Array of pointers */
  void* GTY((skip)) pointers[3];
  
  /* Callback */
  CompareFunc GTY((skip)) sorter;
  
  /* Nested struct with union */
  struct GTY(()) {
    union GTY(()) {
      int as_int;
      float as_float;
    } value;
    int type;
  } nested;
  
  /* Language struct */
  TransactionSafeStruct GTY((skip)) safe_data;
};

/* Union containing everything */
union GTY(()) MegaUnion {
  int scalar;
  SimpleStruct as_struct;
  Point as_point;
  DataUnion as_data_union;
  ComplexType* GTY((skip)) as_complex_ptr;
  int (*GTY((skip)) as_func)(void);
  TransactionSafeStruct as_safe_struct;
};

/* ========== FUNCTION DECLARATIONS ========== */
/* Functions that use the types to prevent dead code elimination */

static void USED init_types(void) {
  /* Force references to all types */
  static SimpleStruct ss = {1, 2.0f};
  static Point p = {10, 20};
  static DataUnion du;
  static ComplexType ct;
  static TransactionSafeStruct tss = {42, NULL};
  
  du.as_int = 100;
  ct.id = 1;
  ct.name = "test";
  ct.sorter = NULL;
}

/* Callback function implementations */
static int USED sample_compare(const void* a, const void* b) {
  return *(const int*)a - *(const int*)b;
}

static void USED sample_callback(void* data) {
  *(int*)data += 1;
}

/* Main function to ensure the file is compilable */
int main(int argc, char** argv) {
  init_types();
  
  /* Use some types to avoid optimization */
  ComplexType local_ct;
  local_ct.id = argc;
  local_ct.name = argv[0];
  
  DataUnion local_du;
  local_du.as_float = 3.14f;
  
  /* Test callback */
  int x = 5;
  sample_callback(&x);
  
  return 0;
}

/* Global instances with GTY markers */
static SimpleStruct GTY(()) global_struct = {0, 0.0f};
static Point GTY(()) global_point = {0, 0};
static DataUnion GTY(()) global_union;
static ComplexType* GTY((skip)) global_complex_ptr = NULL;
static EventHandler GTY(()) global_handler = {"test", sample_callback, NULL};

/* Array of structs */
static SimpleStruct GTY(()) struct_array_global[3] = {
  {1, 1.0f},
  {2, 2.0f},
  {3, 3.0f}
};

/* Make sure gengtype sees pointers in different contexts */
struct GTY(()) PointerNetwork {
  ComplexType* GTY((skip)) node1;
  struct PointerNetwork* GTY((skip)) next;
  void* GTY((skip)) data[2];
};

/* Final type to ensure all categories are covered */
struct GTY(()) TypeCoverageVerifier {
  /* TYPE_SCALAR */
  int scalar_field;
  
  /* TYPE_STRING */
  const char* GTY((skip)) string_field;
  
  /* TYPE_STRUCT */
  SimpleStruct struct_field;
  
  /* TYPE_USER_STRUCT */
  Point user_struct_field;
  
  /* TYPE_UNION */
  DataUnion union_field;
  
  /* TYPE_POINTER */
  void* GTY((skip)) pointer_field;
  
  /* TYPE_ARRAY */
  int array_field[4];
  
  /* TYPE_CALLBACK */
  int (*GTY((skip)) callback_field)(void);
  
  /* TYPE_LANG_STRUCT */
  TransactionSafeStruct GTY((skip)) lang_struct_field;
  
  /* TYPE_UNDEFINED (through pointer) */
  struct OpaqueStruct* GTY((skip)) opaque_field;
};
