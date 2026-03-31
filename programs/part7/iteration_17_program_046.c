/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Basic scalar types */
int global_int;               /* TYPE_SCALAR */
float global_float;           /* TYPE_SCALAR */
double global_double;         /* TYPE_SCALAR */
char global_char;             /* TYPE_SCALAR */

/* String type */
const char *global_string = "test string";  /* TYPE_STRING */

/* Simple struct */
struct GTY(()) SimpleStruct {  /* TYPE_STRUCT */
  int field1;
  float field2;
};

/* User struct (typedef struct) */
typedef struct GTY(()) {       /* TYPE_USER_STRUCT */
  int data;
  char * GTY((skip)) name;
} UserStruct;

/* Union type */
union GTY(()) TestUnion {      /* TYPE_UNION */
  int as_int;
  float as_float;
  struct SimpleStruct * GTY((skip)) as_struct;
};

/* Pointer types */
int *global_int_ptr;           /* TYPE_POINTER */
struct SimpleStruct * GTY((skip)) global_struct_ptr;  /* TYPE_POINTER */
void (*global_func_ptr)(void); /* TYPE_POINTER (to function) */

/* Array types */
int global_int_array[10];      /* TYPE_ARRAY */
float global_float_array[] = {1.0, 2.0, 3.0};  /* TYPE_ARRAY */

/* Complex nested array of pointers */
struct SimpleStruct * GTY((skip)) global_struct_ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */

/* Callback types */
typedef void (*CallbackFunc)(int, const char *);  /* TYPE_CALLBACK */
typedef int (*ComplexCallback)(struct SimpleStruct *, UserStruct *);  /* TYPE_CALLBACK */

/* Language-specific struct with GCC attributes */
struct GTY(()) LangStruct __attribute__((aligned(16))) {  /* TYPE_LANG_STRUCT */
  int data;
  void * GTY((skip)) ptr;
};

/* More complex nested types */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  union TestUnion * GTY((skip)) union_array[4];  /* TYPE_ARRAY of TYPE_POINTER to TYPE_UNION */
  CallbackFunc callback;                         /* TYPE_CALLBACK */
  int scalar_field;                              /* TYPE_SCALAR */
};

/* Union containing struct and callback pointer */
union GTY(()) ComplexUnion {
  struct ContainerStruct container;              /* TYPE_STRUCT */
  CallbackFunc func_ptr;                         /* TYPE_CALLBACK */
  struct LangStruct * GTY((skip)) lang_struct_ptr; /* TYPE_POINTER to TYPE_LANG_STRUCT */
};

/* Typedef for function pointer returning pointer to array */
typedef int (* (*ComplexFuncPtr)(void))[10];     /* TYPE_CALLBACK returning TYPE_POINTER to TYPE_ARRAY */

/* Another level of nesting: struct with pointer to array of function pointers */
struct GTY(()) UltraNested {
  ComplexFuncPtr (*func_array[3])(void);         /* TYPE_ARRAY of TYPE_POINTER to TYPE_CALLBACK */
  union ComplexUnion nested_union;               /* TYPE_UNION */
  struct OpaqueStruct * GTY((skip)) opaque_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Global variables using our complex types */
struct ContainerStruct GTY((skip)) global_container;
union ComplexUnion GTY((skip)) global_complex_union;
struct UltraNested * GTY((skip)) global_ultra_nested;

/* Function using various types (prevents dead code elimination) */
void GTY((skip)) use_types(void) {
  /* Reference all global variables to ensure they're used */
  global_int = 1;
  global_float = 2.0f;
  global_double = 3.0;
  global_char = 'A';
  
  if (global_string[0]) {
    /* Do nothing, just reference */
  }
  
  if (global_int_ptr) {
    *global_int_ptr = 42;
  }
  
  for (int i = 0; i < 10; i++) {
    global_int_array[i] = i;
  }
  
  if (global_func_ptr) {
    /* Can't call without definition, but can check */
  }
  
  /* Initialize some structs */
  if (global_struct_ptr) {
    global_struct_ptr->field1 = 1;
    global_struct_ptr->field2 = 2.0f;
  }
  
  /* Reference the container */
  global_container.scalar_field = 100;
  
  /* Reference the complex union */
  global_complex_union.func_ptr = 0;
  
  /* Reference the ultra nested pointer */
  if (global_ultra_nested) {
    /* Access nested member */
    global_ultra_nested->nested_union.container.scalar_field = 200;
  }
}

/* Callback function definitions */
static void simple_callback(int x, const char *str) {
  /* Empty implementation */
}

static int complex_callback(struct SimpleStruct *s, UserStruct *u) {
  return s ? s->field1 : 0;
}

/* Main function for standalone compilation */
int main(void) {
  /* Initialize function pointers */
  global_func_ptr = 0;  /* Would need actual function to assign */
  
  /* Create some instances on stack */
  struct SimpleStruct local_struct = {1, 2.0f};
  UserStruct local_user_struct = {42, 0};
  union TestUnion local_union;
  
  /* Use the types */
  use_types();
  
  /* Test callbacks */
  CallbackFunc cb = simple_callback;
  ComplexCallback ccb = complex_callback;
  
  /* Call if non-null */
  if (cb) cb(1, "test");
  if (ccb) ccb(&local_struct, &local_user_struct);
  
  return 0;
}

/* Additional undefined type (forward declared but never defined) */
struct AnotherOpaque;

/* Array of undefined type pointers */
struct AnotherOpaque * GTY((skip)) opaque_array[3];

/* Mixed array with various types */
void * GTY((skip)) mixed_array[] = {
  &global_int,
  global_string,
  &global_container,
  &global_complex_union
};

/* Function with array parameter */
void process_array(int arr[]) {  /* TYPE_ARRAY as parameter */
  if (arr) arr[0] = 0;
}

/* Struct with flexible array member */
struct GTY(()) FlexStruct {
  int count;
  int data[];  /* TYPE_ARRAY (flexible array member) */
};
