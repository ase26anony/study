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
/* String type - char* with string context */
static const char* GTY(()) string_literal = "test string";
char* GTY(()) global_string_ptr;

/* ========== TYPE_STRUCT ========== */
/* Plain struct types */
struct GTY(()) SimpleStruct {
  int x;
  float y;
};

struct GTY(()) NestedStruct {
  SimpleStruct inner;
  double z;
};

/* Struct with bitfields */
struct GTY(()) BitfieldStruct {
  unsigned int a : 4;
  unsigned int b : 8;
  unsigned int c : 16;
};

/* ========== TYPE_USER_STRUCT ========== */
/* Typedef struct combinations */
typedef struct GTY(()) {
  int id;
  char name[32];
} UserStruct;

typedef struct GTY(()) TaggedStruct {
  int tag;
  union {
    int ival;
    float fval;
    char* sval;
  } data;
} *TaggedStructPtr;

/* Another user struct with nested typedef */
typedef struct GTY(()) OuterStruct {
  UserStruct inner;
  TaggedStructPtr ptr;
} OuterStruct;

/* ========== TYPE_UNION ========== */
/* Union types */
union GTY(()) SimpleUnion {
  int i;
  float f;
  double d;
  char* s;
};

/* Union within struct */
struct GTY(()) StructWithUnion {
  int type;
  union GTY(()) {
    int int_val;
    float float_val;
    void* ptr_val;
  } value;
};

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* GTY(()) global_int_ptr;
SimpleStruct* GTY(()) global_struct_ptr;
UserStruct** GTY(()) global_double_ptr;
const char* GTY(()) global_const_ptr;
volatile int* GTY(()) global_volatile_ptr;

/* Pointer in struct */
struct GTY(()) PointerStruct {
  int* GTY((skip)) raw_ptr;  /* skip for GC */
  SimpleStruct* GTY(()) gc_ptr;
  void (*GTY(()) callback)(int);
};

/* ========== TYPE_ARRAY ========== */
/* Array types */
int GTY(()) fixed_array[10];
SimpleStruct GTY(()) struct_array[5];
UserStruct* GTY(()) pointer_array[8];

/* Array with unspecified size */
extern int GTY(()) extern_array[];

/* Multi-dimensional array */
int GTY(()) matrix[3][4];

/* Array in struct */
struct GTY(()) ArrayStruct {
  int counts[20];
  char* GTY(()) strings[5];
  SimpleStruct items[];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef void (*GTY(()) SimpleCallback)(int, char*);
typedef int (*GTY(())) ComplexCallback(SimpleStruct*, UserStruct**);

/* Callback returning pointer */
typedef SimpleStruct* (*GTY(())) StructFactory(int id);

/* Callback taking callback */
typedef void (*GTY(())) EventHandler(SimpleCallback cb, void* data);

/* Struct with callback members */
struct GTY(()) CallbackContainer {
  SimpleCallback on_start;
  ComplexCallback* on_process;
  EventHandler on_event;
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structs using GCC extensions */
struct GTY(()) TransactionSafeStruct {
  int value;
  char* name;
} __attribute__((transaction_safe));

struct GTY(()) AlignedStruct {
  double data;
  int tag;
} __attribute__((aligned(16)));

/* Vector type (GCC extension) */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) VectorStruct {
  v4si vectors[4];
  int count;
};

/* ========== COMPLEX NESTED TYPES ========== */
/* Deeply nested type combining multiple categories */
typedef union GTY(()) PolyValue {
  int as_int;
  float as_float;
  char* as_string;
  SimpleStruct* as_struct;
  void (*as_func)(void);
} PolyValue;

struct GTY(()) ComplexNode {
  int type_id;
  PolyValue value;
  struct ComplexNode* GTY(()) next;
  struct ComplexNode* GTY(()) children[4];
  void (*GTY(()) processor)(struct ComplexNode*);
};

typedef struct GTY(()) {
  ComplexNode* GTY(()) root;
  UserStruct* GTY(()) users[10];
  SimpleCallback GTY(()) callbacks[3];
  union {
    int mode;
    float threshold;
  } config;
} ApplicationState;

/* ========== TEMPLATE-LIKE STRUCTURES ========== */
/* Simulating template patterns */
struct GTY(()) GenericContainer {
  void* GTY((skip)) data;  /* type-erased pointer */
  size_t size;
  void (*GTY(()) destructor)(void*);
  int (*GTY(()) comparator)(const void*, const void*);
};

/* ========== VARIABLE DECLARATIONS ========== */
/* Global instances to ensure types are used */
SimpleStruct GTY(()) global_struct_instance = {1, 2.0f};
UserStruct GTY(()) global_user_struct = {42, "test"};
SimpleUnion GTY(()) global_union_instance;
ArrayStruct* GTY(()) global_array_struct;
ComplexNode* GTY(()) global_node_chain;
ApplicationState GTY(()) app_state;
TransactionSafeStruct GTY(()) ts_struct = {100, "transaction"};
VectorStruct GTY(()) vec_struct;

/* ========== FUNCTION DEFINITIONS ========== */
/* Functions that use the types */
static void USED simple_callback_impl(int x, char* s) {
  /* Do nothing, just for type reference */
  (void)x;
  (void)s;
}

static int USED complex_callback_impl(SimpleStruct* s, UserStruct** u) {
  if (s && u) return s->x;
  return 0;
}

static void USED process_node(ComplexNode* node) {
  if (!node) return;
  node->type_id = 1;
}

/* Initialize global instances */
static void USED init_globals(void) {
  global_union_instance.i = 42;
  global_string_ptr = (char*)string_literal;
  
  /* Create a simple node chain */
  static ComplexNode node1, node2;
  node1.type_id = 1;
  node1.value.as_int = 100;
  node1.next = &node2;
  node1.processor = process_node;
  
  node2.type_id = 2;
  node2.value.as_float = 3.14f;
  node2.next = NULL;
  node2.processor = process_node;
  
  global_node_chain = &node1;
  
  /* Initialize vector struct */
  for (int i = 0; i < 4; i++) {
    vec_struct.vectors[i] = (v4si){i, i+1, i+2, i+3};
  }
  vec_struct.count = 4;
}

/* Main test function */
void USED gt_test_function(void) {
  init_globals();
  
  /* Use various types to prevent optimization */
  SimpleStruct local_struct = {global_struct_instance.x + 1, 3.0f};
  UserStruct* user_ptr = &global_user_struct;
  int* int_ptr = &local_struct.x;
  
  /* Array access */
  for (int i = 0; i < 10; i++) {
    fixed_array[i] = i * 2;
  }
  
  /* Callback usage */
  SimpleCallback cb = simple_callback_impl;
  if (cb) cb(42, "test");
  
  /* Union access */
  SimpleUnion u;
  u.i = 100;
  u.f = (float)u.i;
  
  /* Pointer operations */
  TaggedStructPtr tagged = (TaggedStructPtr)int_ptr;
  if (global_struct_ptr) {
    global_struct_ptr->x = 50;
  }
}

/* ========== MAIN FOR SOURCE VALIDITY ========== */
#ifdef STANDALONE_TEST
/* Only compile main if building standalone test */
int main(void) {
  gt_test_function();
  return 0;
}
#endif
