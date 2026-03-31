/* complex_gty_test.c - Test file for gengtype delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter list */
typedef void (*GTY((skip)) complex_func_ptr)(int (*GTY((skip)) callback)(char[10]), 
                                             struct GTY((skip)) { int x; } param);

struct GTY(()) TestStruct1 {
  complex_func_ptr GTY((skip)) func_field;
  
  /* Array with complex size expression */
  int GTY((skip)) arr[(10 + sizeof(struct { char c; int i; }))];
};

/* Test 2: Deeply nested parentheses in function pointer type */
typedef int (*(*GTY((skip)) nested_func_ptr[5])(void))(char *(*(*)(int))[10]);

union GTY(()) TestUnion1 {
  /* Chain of pointers with attributes */
  struct GTY((chain_next, chain_prev)) TestStruct1 *GTY((tag("0"))) next;
  
  /* Complex array of function pointers */
  void (*(*GTY((skip)) fn_matrix[3][4])(int, ...))(double);
};

/* Test 3: Multiple delimiter types in single declaration */
struct GTY(()) TestStruct2 {
  /* Combined parentheses, brackets, and braces */
  struct {
    int (*GTY((skip)) callback)(int[][10]);
    union {
      char *(*GTY((skip)) name_func)(void);
      int data[5];
    } GTY((skip)) u;
  } GTY((skip)) nested;
  
  /* Initializer-like brace structure in type definition */
  enum { RED, GREEN, BLUE } (*GTY((skip)) color_array[3])(struct { int r, g, b; });
};

/* Test 4: Template-like macro with nested delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
  struct GTY(()) vector_##TYPE { \
    TYPE GTY((skip)) data[SIZE]; \
    int (*GTY((skip)) compare)(TYPE *, TYPE *); \
  }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(struct TestStruct1 *, 5);

/* Test 5: Attribute list with nested parentheses */
struct GTY((chain_next("next"), chain_prev("prev"),
           length("count"), variable_size)) TestStruct3 {
  int GTY((skip)) count;
  struct TestStruct3 *GTY((skip)) next;
  struct TestStruct3 *GTY((skip)) prev;
  
  /* Multi-dimensional array with function pointer elements */
  void (*(*GTY((skip)) operations[2][3])(int))(float) GTY((skip));
  
  /* Nested anonymous struct with bitfield */
  struct {
    unsigned int GTY((skip)) flags : 4;
    int (*GTY((skip)) handler)(struct { int a; double b; } *);
  } GTY((skip)) state;
};

/* Test 6: Recursive type definition with all delimiters */
typedef struct GTY(()) TreeNode {
  char *GTY((skip)) name;
  struct TreeNode *GTY((skip)) children[10];
  
  /* Function pointer returning array pointer */
  int (*(*GTY((skip)) get_values)(void))[10];
  
  /* Union with nested struct initializer pattern */
  union {
    struct {
      int (*GTY((skip)) validate)(int (*)(char), double);
      char buffer[256];
    } GTY((skip)) data;
    long GTY((skip)) raw[64];
  } GTY((skip)) payload;
} TreeNode;

/* Test 7: Complex typedef with multiple nested levels */
typedef int (*(*(*GTY((skip)) ultra_complex_type)(int (*[5])(double)))[10])(char);

/* Test 8: Structure with designated initializer-like members */
struct GTY(()) Config {
  /* Array with nested struct type */
  struct {
    const char *GTY((skip)) name;
    int (*GTY((skip)) init)(void);
    void (*GTY((skip)) cleanup)(int, ...);
  } GTY((skip)) modules[5];
  
  /* Pointer to function returning pointer to array */
  int (*(*GTY((skip)) get_matrix)(int size))[10][10];
  
  /* Anonymous union with function pointer array */
  union {
    void (*GTY((skip)) actions[3])(struct Config *);
    struct {
      int (*GTY((skip)) step)(int[3][3]);
      double (*GTY((skip)) transform)(float (*)(int), char);
    } GTY((skip)) ops;
  } GTY((skip)) methods;
};

/* Test 9: Multiple GTY annotations in nested contexts */
struct GTY(()) Outer {
  struct GTY((for_user)) Inner {
    /* Function pointer with GTY attributes on parameters */
    void (*GTY((callback)) on_event)(
      struct Inner *GTY((skip)) self,
      int GTY((skip)) data[][10],
      void (*GTY((skip)) completion)(struct { int status; } *)
    );
    
    /* Array of structures containing function pointers */
    struct {
      int (*GTY((skip)) compare)(const void *, const void *);
      void (*GTY((skip)) swap)(void *, void *);
    } GTY((skip)) operations[2];
  } *GTY((skip)) inner;
  
  /* Pointer to array of function pointers */
  void (*(*GTY((skip)) callbacks[5])(int))(void);
};

/* Test 10: Final comprehensive test with all delimiter types */
struct GTY(()) ComprehensiveTest {
  /* Parentheses: function pointer */
  int (*GTY((skip)) func1)(int, char *(*)(void));
  
  /* Brackets: multi-dimensional array */
  float GTY((skip)) matrix[3][(sizeof(int) + 5)][10];
  
  /* Braces: nested anonymous struct */
  struct {
    /* All three in one member */
    void (*(*GTY((skip)) complex_array[2])(int[][5]))(struct { int x; } *);
    
    /* Initializer-like type */
    enum { 
      MODE_A = 1, 
      MODE_B = 2, 
      MODE_C = 3 
    } (*GTY((skip)) mode_switch)(int, ...);
  } GTY((skip)) container;
  
  /* Chain of all three */
  struct GTY((chain_next)) ComprehensiveTest *GTY((skip)) next;
};

/* Dummy main to make file compilable */
int main(void) {
  return 0;
}
