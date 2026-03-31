/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built with GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

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
typedef const char * GTY(()) string_ptr_t;

/* Struct types (TYPE_STRUCT) */
struct GTY(()) SimpleStruct {
  scalar_int_t field1;
  scalar_float_t field2;
};

/* User struct types (TYPE_USER_STRUCT) */
typedef struct GTY(()) {
  scalar_int_t data;
  scalar_char_t flag;
} UserStruct;

typedef struct GTY(()) ComplexUserStruct {
  UserStruct base;
  scalar_double_t extra;
} ComplexUserStruct;

/* Union types (TYPE_UNION) */
union GTY(()) SimpleUnion {
  scalar_int_t as_int;
  scalar_float_t as_float;
  scalar_char_t as_char;
};

/* Pointer types (TYPE_POINTER) */
typedef scalar_int_t * GTY(()) int_ptr_t;
typedef SimpleStruct * GTY(()) struct_ptr_t;
typedef UserStruct * GTY(()) user_struct_ptr_t;
typedef void (* GTY(()) void_func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef scalar_int_t GTY(()) int_array_10_t[10];
typedef scalar_char_t GTY(()) char_array_t[];
typedef UserStruct GTY(()) user_struct_array_t[5];

/* Callback types (TYPE_CALLBACK) */
typedef scalar_int_t (* GTY(()) callback_func_t)(scalar_int_t, scalar_float_t);
typedef void (* GTY(()) complex_callback_t)(struct_ptr_t, user_struct_ptr_t);

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct GTY(()) LangStruct {
  scalar_int_t data;
} __attribute__((transaction_safe));

struct GTY(()) AnotherLangStruct {
  scalar_double_t value;
} __attribute__((aligned(16)));

/* Complex nested types to ensure deep traversal */

/* Struct containing array of pointers to unions */
struct GTY(()) ContainerStruct {
  union GTY(()) NestedUnion {
    scalar_int_t i;
    scalar_float_t f;
    struct GTY(()) TinyStruct {
      scalar_char_t c;
    } ts;
  } GTY((tag("0"))) data[4];
  
  SimpleUnion * GTY((skip)) union_ptrs[3];
  int_array_10_t fixed_array;
};

/* Union containing struct and callback pointer */
union GTY(()) MegaUnion {
  struct GTY(()) InternalStruct {
    UserStruct user;
    ComplexUserStruct complex;
  } internal;
  
  callback_func_t callback;
  void * GTY((skip)) opaque;
};

/* Typedef for function pointer returning pointer to array */
typedef scalar_int_t (* GTY(()) (*complex_func_ptr_t)(void))[10];

/* Struct with all kinds of members */
struct GTY(()) UltimateStruct {
  /* Scalars */
  scalar_int_t count;
  scalar_float_t ratio;
  scalar_double_t precision;
  scalar_char_t initial;
  scalar_bool_t enabled;
  
  /* String */
  string_ptr_t GTY((length("strlen(%h.name)"))) name;
  
  /* Structs */
  SimpleStruct simple;
  UserStruct user;
  
  /* Unions */
  SimpleUnion choice;
  
  /* Pointers */
  int_ptr_t numbers;
  struct_ptr_t next;
  OpaqueStruct * GTY((skip)) opaque_ptr;
  
  /* Arrays */
  int_array_10_t scores;
  char_array_t GTY((length("count"))) buffer;
  
  /* Callbacks */
  callback_func_t processor;
  complex_callback_t notifier;
  
  /* Language struct */
  LangStruct lang_data;
  
  /* Nested complex types */
  ContainerStruct container;
  MegaUnion mega;
};

/* Global instances to ensure types are used */
static UltimateStruct GTY(()) global_ultimate;
static ContainerStruct GTY(()) global_container;
static MegaUnion GTY(()) global_mega;
static LangStruct GTY(()) global_lang;
static UserStruct GTY(()) global_user_structs[3];

/* Function using the types */
void GTY((user)) gt_test_function(void)
{
  /* Reference globals to prevent elimination */
  global_ultimate.count = 42;
  global_container.data[0].i = 100;
  global_mega.internal.user.data = 200;
  global_lang.data = 300;
  global_user_structs[0].data = 400;
  
  /* Create local instances */
  SimpleStruct local_simple = {1, 2.0f};
  UserStruct local_user = {5, 'A'};
  SimpleUnion local_union;
  local_union.as_int = 10;
  
  /* Use pointers */
  int_ptr_t local_ptr = &global_ultimate.count;
  *local_ptr = 999;
  
  /* Use array */
  global_ultimate.scores[0] = 100;
  
  /* String assignment */
  global_ultimate.name = "Test String";
}

/* Another function with different type usage */
static void process_with_callback(callback_func_t func)
{
  if (func) {
    scalar_int_t result = func(10, 3.14f);
    global_ultimate.count += result;
  }
}

/* Main function for standalone compilation */
#ifdef STANDALONE_TEST
int main(void)
{
  gt_test_function();
  
  /* Test callback */
  auto test_callback = [](scalar_int_t a, scalar_float_t b) -> scalar_int_t {
    return a + (scalar_int_t)b;
  };
  
  process_with_callback(test_callback);
  return 0;
}
#endif

/* Additional forward declarations to ensure TYPE_UNDEFINED */
struct GTY(()) AnotherOpaque;
union GTY(()) YetAnotherOpaque;
class GTY(()) OpaqueClass;  /* C++ class forward declaration */
