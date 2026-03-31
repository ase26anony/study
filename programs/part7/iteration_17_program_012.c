/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

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
typedef const char *GTY(()) string_ptr_t;

/* Basic struct (TYPE_STRUCT) */
struct GTY(()) BasicStruct {
  int field1;
  float field2;
};

/* User struct via typedef (TYPE_USER_STRUCT) */
typedef struct GTY(()) {
  int x;
  double y;
  char *GTY((skip)) name;  /* Skip this for GC */
} UserStruct;

/* Another struct with complex members */
struct GTY(()) ComplexStruct {
  int *GTY((skip)) data;           /* Pointer (not GC-tracked) */
  struct BasicStruct *GTY(()) bs;  /* Pointer to another struct */
};

/* Union type (TYPE_UNION) */
union GTY(()) DataUnion {
  int int_val;
  float float_val;
  double double_val;
  char *GTY((skip)) string_val;
  struct BasicStruct *GTY(()) struct_val;
};

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) IntArray[10];
typedef struct BasicStruct GTY(()) StructArray[5];
typedef union DataUnion GTY(()) UnionArray[3];

/* Pointer types (TYPE_POINTER) */
typedef int *GTY(()) IntPtr;
typedef struct BasicStruct *GTY(()) StructPtr;
typedef union DataUnion *GTY(()) UnionPtr;
typedef void *GTY(()) VoidPtr;

/* Callback/function types (TYPE_CALLBACK) */
typedef int (*GTY(()) SimpleCallback)(int, float);
typedef void (*GTY(()) ComplexCallback)(struct BasicStruct *GTY(()), 
                                        union DataUnion *GTY(()));

/* Function pointer returning pointer to array */
typedef int (*GTY(()) CallbackReturningArrayPtr)(void)[10];

/* Struct containing function pointer */
struct GTY(()) StructWithCallback {
  int id;
  SimpleCallback GTY((skip)) callback;  /* Function pointer */
  ComplexCallback GTY((skip)) complex_cb;
};

/* Language-specific struct (TYPE_LANG_STRUCT) */
/* Use GCC attributes to create language-specific variants */
struct GTY(()) LangStruct1 {
  int data;
} __attribute__((aligned(16)));

struct GTY(()) LangStruct2 {
  double value;
} __attribute__((packed));

struct GTY(()) LangStruct3 {
  char *GTY((skip)) name;
  int count;
} __attribute__((transaction_safe));

/* Nested complex type definitions */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  int count;
  union DataUnion *GTY(()) items[10];  /* Array of pointers to unions */
  struct BasicStruct GTY(()) base;     /* Embedded struct */
};

/* Union containing struct and callback pointer */
union GTY(()) MegaUnion {
  struct ContainerStruct GTY(()) container;
  struct StructWithCallback GTY(()) with_callback;
  CallbackReturningArrayPtr GTY((skip)) array_callback;
};

/* Typedef for function pointer returning pointer to array */
typedef int (*GTY(()) ComplexFuncPtr)(void)[10];

/* Another level of nesting */
struct GTY(()) UltimateStruct {
  struct ContainerStruct *GTY(()) container_ptr;
  union MegaUnion GTY(()) mega_union;
  IntArray GTY(()) numbers;
  string_ptr_t GTY(()) description;
  LangStruct1 GTY(()) lang_member;
};

/* Global variables to instantiate types */
scalar_int_t GTY(()) global_int = 42;
scalar_float_t GTY(()) global_float = 3.14f;
string_ptr_t GTY(()) global_string = "Hello, gengtype!";
struct BasicStruct GTY(()) global_basic_struct = {1, 2.0f};
UserStruct GTY(()) global_user_struct = {10, 20.5, NULL};
union DataUnion GTY(()) global_union;
IntArray GTY(()) global_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
struct ContainerStruct GTY(()) global_container;
struct UltimateStruct GTY(()) global_ultimate;

/* Function using the types */
void GTY((user)) init_types(void) {
  /* Initialize union */
  global_union.int_val = 100;
  
  /* Initialize container */
  global_container.count = 3;
  global_container.base.field1 = 1;
  global_container.base.field2 = 2.0f;
  
  /* Initialize ultimate struct */
  global_ultimate.container_ptr = &global_container;
  global_ultimate.mega_union.container = global_container;
  global_ultimate.description = "Test structure";
  global_ultimate.lang_member.data = 999;
  
  for (int i = 0; i < 10; i++) {
    global_ultimate.numbers[i] = i * 2;
  }
}

/* More complex type combinations */

/* Pointer to array of structs */
typedef struct BasicStruct (*GTY(())) StructArrayPtr[5];

/* Array of function pointers */
typedef int (*GTY(()) FuncPtrArray[5])(int, int);

/* Struct with all type categories */
struct GTY(()) AllTypesStruct {
  /* Scalar */
  int GTY(()) scalar_int;
  float GTY(()) scalar_float;
  
  /* String */
  const char *GTY(()) string_field;
  
  /* Struct */
  struct BasicStruct GTY(()) embedded_struct;
  
  /* Union */
  union DataUnion GTY(()) embedded_union;
  
  /* Pointer */
  struct ContainerStruct *GTY(()) struct_ptr;
  
  /* Array */
  int GTY(()) int_array[5];
  
  /* Callback */
  int (*GTY((skip))) callback_field)(int);
  
  /* Language struct */
  LangStruct2 GTY(()) lang_field;
};

/* Additional undefined types for TYPE_UNDEFINED */
class GTY(()) UndefinedClass;      /* C++ class forward declaration */
enum GTY(()) UndefinedEnum;        /* Enum forward declaration */

/* Template for C++ (if building as C++) */
#ifdef __cplusplus
template<typename T>
struct GTY(()) TemplateStruct {
  T GTY(()) data;
  T *GTY(()) ptr;
};

/* Instantiate template */
TemplateStruct<int> GTY(()) global_template_int;
#endif

/* Main function to ensure the file is compilable */
int main(int argc, char **argv) {
  init_types();
  return 0;
}
