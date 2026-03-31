/* complex_gty_test.c - Test file to exercise gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with deeply nested parameter lists */
typedef int (*GTY(()) complex_callback_t)(
    char *GTY((skip)) buffer[256],
    void (*GTY(()) nested_fn)(int, struct GTY(()) { int x; double y; })
);

/* Test 2: Structure with multiple nested delimiter types */
struct GTY(()) OuterStruct {
  /* Array with complex size expression containing parentheses */
  int arr1[(10 + sizeof(struct GTY(()) Inner { int a; char b; }))];
  
  /* Function pointer array with nested brackets and parentheses */
  void (*GTY((chain_next, chain_prev)) fn_array[5])(
    int matrix[][10],
    struct GTY(()) { 
      int count; 
      char *GTY((length("count"))) data; 
    } param
  );
  
  /* Nested union with bit-fields and array */
  union GTY(()) {
    struct GTY(()) {
      unsigned int flags : 4;
      int values[3];
    } bits;
    long long combined;
  } GTY((tag("union_type"))) nested_union;
};

/* Test 3: Template-like pattern using nested structures */
struct GTY(()) Container {
  /* Pointer to function returning pointer to array */
  int (*(*GTY(()) complex_func_ptr)(void))[10];
  
  /* Structure containing array of function pointers */
  struct GTY(()) FuncTable {
    void (*handlers[8])(int, ...);
    int (*validator)(char *GTY((string)) str);
  } GTY((desc("%1.func_table"))) func_table;
  
  /* Anonymous struct with initializer-like designators */
  struct GTY(()) {
    int x;
    struct GTY(()) {
      double y;
      float z[2][2];
    } inner;
  } anonymous;
};

/* Test 4: Multiple balanced delimiters in single declaration */
typedef void (*(*GTY(()) ultra_complex_t)[5])(
  int arg1,
  char *GTY((length("arg1"))) arg2[],
  struct GTY(()) {
    int tag;
    union GTY(()) {
      int i;
      double d;
      void *p;
    } value;
  } arg3
);

/* Test 5: Structure with complex initializer (triggers brace handling) */
static struct GTY(()) InitializedStruct {
  int id;
  char *name;
  struct GTY(()) {
    int x;
    int y;
    int z[3];
  } coordinates;
} GTY(()) global_instance = {
  42,
  "test",
  { 
    100, 
    200, 
    { 1, 2, 3 }  /* Nested braces inside braces */
  }
};

/* Test 6: Union with nested attribute lists */
union GTY((desc("%0.union_type"), tag("union_tag"))) ComplexUnion {
  struct GTY(()) {
    int type;
    void (*callback)(int, ...);
  } handler;
  
  struct GTY(()) {
    int count;
    /* Array with designators in initializer */
    struct GTY(()) Element {
      int id;
      char data[64];
    } elements[10];
  } container;
};

/* Test 7: Typedef chain with multiple GTY annotations */
typedef struct GTY(()) BaseType {
  int base_field;
} BaseType_t;

typedef BaseType_t *GTY((ptr_alias)) BasePtr_t;

typedef BasePtr_t (*GTY(()) GetterFunc)(
  int index,
  BaseType_t (*GTY(()) factories[])(void)
);

/* Test 8: Structure containing all delimiter types mixed */
struct GTY(()) AllDelimiters {
  /* Parentheses in function type */
  int (*compare)(const void *, const void *);
  
  /* Brackets in array with computed size */
  unsigned char buffer[sizeof(struct GTY(()) { int a; double b; })];
  
  /* Braces for nested anonymous struct */
  struct GTY(()) {
    /* Nested parentheses in function pointer array */
    void (*actions[3])(void);
    
    /* Nested brackets in multidimensional array */
    int matrix[2][(5 + 1)];
    
    /* Nested braces for union */
    union GTY(()) {
      int i;
      float f[2];
    } value;
  } nested;
};

/* Test 9: Recursive structure with function pointers */
struct GTY(()) TreeNode {
  char *GTY((string)) data;
  struct GTY((chain_next)) TreeNode *next;
  struct GTY((chain_prev)) TreeNode *prev;
  
  /* Function pointer with complex return type */
  struct GTY(()) Result *(*processor)(
    int mode,
    char args[][32],
    void (*GTY(()) completion)(struct GTY(()) Result *)
  );
};

/* Test 10: Macro-like constructs that expand to delimiter-heavy types */
#define DECLARE_CALLBACK(name, ret, ...) \
  typedef ret (*GTY(()) name##_t)(__VA_ARGS__)

/* This expands to: typedef void (*GTY(()) callback_t)(int, char *) */
DECLARE_CALLBACK(callback, void, int, char *);

/* Complex expansion with nested delimiters */
#define DECLARE_ARRAY_FUNC(name, size) \
  typedef void (*name##_array[size])(int (*)(char[10]))

DECLARE_ARRAY_FUNC(ComplexFunc, 5);

/* Main function to make the file compilable */
int main(void) {
  /* Access structures to avoid unused warnings */
  struct OuterStruct os = {0};
  struct Container c = {0};
  struct AllDelimiters ad = {0};
  
  (void)os;
  (void)c;
  (void)ad;
  (void)global_instance;
  
  return 0;
}
