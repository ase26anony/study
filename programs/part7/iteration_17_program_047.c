/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;           /* TYPE_UNDEFINED */
union OpaqueUnion;             /* TYPE_UNDEFINED */

/* Basic scalar types */
typedef int scalar_int;        /* TYPE_SCALAR */
typedef float scalar_float;    /* TYPE_SCALAR */
typedef double scalar_double;  /* TYPE_SCALAR */
typedef char scalar_char;      /* TYPE_SCALAR */

/* String type - char* used as string */
typedef char* string_type;     /* TYPE_STRING */

/* Regular struct types */
struct GTY(()) SimpleStruct {  /* TYPE_STRUCT */
  int field1;
  float field2;
};

/* User struct types (typedef struct) */
typedef struct GTY(()) {       /* TYPE_USER_STRUCT */
  int data;
  char* GTY((skip)) name;
} UserStruct;

/* Another struct with complex members */
struct GTY(()) ComplexStruct {
  int* GTY((skip)) int_ptr;    /* TYPE_POINTER */
  struct SimpleStruct* GTY((skip)) nested_struct;
  void (*callback)(int);       /* TYPE_CALLBACK */
};

/* Union type */
union GTY(()) TestUnion {      /* TYPE_UNION */
  int as_int;
  float as_float;
  struct SimpleStruct* GTY((skip)) as_struct;
};

/* Array types */
typedef int IntArray[10];      /* TYPE_ARRAY */
typedef struct SimpleStruct StructArray[5]; /* TYPE_ARRAY */

/* Incomplete array type */
struct GTY(()) WithFlexArray {
  int count;
  int data[];                  /* TYPE_ARRAY (incomplete) */
};

/* Complex nested type with array of pointers */
struct GTY(()) Container {
  union TestUnion* GTY((skip)) union_array[8]; /* TYPE_ARRAY of TYPE_POINTER */
  void (*callbacks[3])(void);  /* TYPE_ARRAY of TYPE_CALLBACK */
  int (*matrix[4][4])(void);   /* TYPE_ARRAY of TYPE_ARRAY of TYPE_CALLBACK */
};

/* Language-specific struct with GCC attributes */
struct GTY(()) LangStruct      /* TYPE_LANG_STRUCT */
  __attribute__((aligned(16)))
  __attribute__((transaction_safe)) {
  int transaction_data;
  void* GTY((skip)) transaction_ptr;
};

/* Complex callback types */
typedef int (*SimpleCallback)(void);                    /* TYPE_CALLBACK */
typedef void (*ComplexCallback)(struct Container*, int); /* TYPE_CALLBACK */
typedef struct SimpleStruct* (*FactoryCallback)(int);   /* TYPE_CALLBACK */

/* Function pointer returning pointer to array */
typedef int (*CallbackReturningArrayPtr)(void)[10];     /* TYPE_CALLBACK */

/* Nested pointer types */
typedef int*** TriplePointer;                          /* TYPE_POINTER */

/* Self-referential structures */
struct GTY(()) TreeNode {
  int value;
  struct TreeNode* GTY((skip)) left;   /* TYPE_POINTER */
  struct TreeNode* GTY((skip)) right;  /* TYPE_POINTER */
};

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  struct {
    int tag;
    union TestUnion data;
  } s;                                 /* TYPE_STRUCT inside union */
  void (*handler)(union MixedUnion*);  /* TYPE_CALLBACK */
};

/* Typedef chain */
typedef int MyInt;
typedef MyInt MyIntAlias;
typedef MyIntAlias* MyIntPtr;          /* TYPE_POINTER */

/* Global variables to instantiate types */
struct SimpleStruct GTY((skip)) global_struct;
UserStruct GTY((skip)) global_user_struct;
union TestUnion GTY((skip)) global_union;
struct Container GTY((skip)) global_container;
struct LangStruct GTY((skip)) global_lang_struct;
IntArray GTY((skip)) global_array = {0};
struct WithFlexArray* GTY((skip)) flex_array_ptr;

/* Function using various types */
void GTY((skip)) use_types(void) {
  /* Reference undefined types */
  extern struct OpaqueStruct* opaque_ptr;
  extern union OpaqueUnion* opaque_union_ptr;
  
  /* Use scalar types */
  scalar_int si = 42;
  scalar_float sf = 3.14f;
  scalar_double sd = 2.71828;
  scalar_char sc = 'A';
  
  /* Use string type */
  string_type str = "Hello, gengtype!";
  
  /* Use pointer types */
  int* int_ptr = &si;
  struct SimpleStruct* struct_ptr = &global_struct;
  TriplePointer triple_ptr = NULL;
  
  /* Use array types */
  IntArray local_array = {1, 2, 3, 4, 5};
  struct SimpleStruct struct_array[2];
  
  /* Use callback types */
  SimpleCallback cb1 = NULL;
  ComplexCallback cb2 = NULL;
  
  /* Use union */
  global_union.as_int = 100;
  
  /* Use nested types */
  global_container.union_array[0] = &global_union;
  
  /* Use language-specific struct */
  global_lang_struct.transaction_data = 999;
  
  /* Prevent unused variable warnings */
  (void)si; (void)sf; (void)sd; (void)sc;
  (void)str; (void)int_ptr; (void)struct_ptr;
  (void)triple_ptr; (void)local_array;
  (void)struct_array; (void)cb1; (void)cb2;
}

/* Dummy main for source validity */
int main(void) {
  use_types();
  return 0;
}

/* Additional undefined type declarations in separate "header" */
#ifdef MAKE_HEADER
/* opaque-types.h */
struct OpaqueStruct;
union OpaqueUnion;
typedef struct OpaqueStruct* OpaqueHandle;
#endif
