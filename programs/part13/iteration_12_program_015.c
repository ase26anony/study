/* complex-gty-test.c - Test file for gengtype delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter list */
typedef void (*GTY((user)) complex_func_ptr)(int (*GTY((skip)) callback)(char[10]), 
                                             struct GTY(()) inner { int x; });

/* Test 2: Array with complex dimension expression */
struct GTY(()) array_test {
  int arr1[(10 + sizeof(struct GTY(()) size_calc { char c; int i; }))];
  char arr2[5][(sizeof(double) * 2)];
};

/* Test 3: Nested structures with all delimiter types */
struct GTY(()) outer_struct {
  /* Function pointer array */
  void (*GTY((chain_next)) fp_array[3])(int[][10]);
  
  /* Anonymous union with bitfields */
  union {
    int GTY((skip)) x:5;
    long GTY((skip)) y:10;
  } GTY((tag("0"))) bits;
  
  /* Pointer to array of function pointers */
  int (*(*GTY((user)) complex_ptr)[5])(char, float);
};

/* Test 4: Deeply nested parentheses in type declaration */
typedef int (*(*GTY((user)) nested_func_ptr)(struct GTY(()) param {
  int depth;
  char name[20];
}))(void (*)(int), char[]);

/* Test 5: Structure with initializer-style nested braces */
struct GTY(()) init_test {
  struct coordinates {
    int x;
    int y;
    int z;
  } GTY((user)) points[3];
  
  /* Complex initializer would be here if allowed in GTY context */
  int matrix[2][2];
};

/* Test 6: Multiple balanced delimiters in single declaration */
struct GTY(()) delimiter_mix {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*GTY((chain_next)) fn_array[5])(int[][10]);
  
  /* Nested structure with array of pointers */
  struct GTY(()) nested {
    int *GTY((skip)) ptr_array[4];
    struct GTY(()) deeper {
      char str[50];
    } GTY((user)) inner;
  } GTY((user)) container;
  
  /* Function returning pointer to array */
  int (*GTY((user)) (*getter)(int index))[10];
};

/* Test 7: Union with variant containing nested types */
union GTY((desc("tag"))) variant_union {
  int GTY((skip)) tag;
  
  struct GTY((tag("1"))) case1 {
    int (*GTY((user)) handler)(char *args[]);
    float matrix[3][3];
  } GTY((user)) data1;
  
  struct GTY((tag("2"))) case2 {
    struct GTY(()) recursive {
      union variant_union *GTY((skip)) next;
      int depth;
    } GTY((user)) chain;
    long values[10];
  } GTY((user)) data2;
};

/* Test 8: Template-like macro expansion with delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
  struct GTY(()) vector_##TYPE { \
    TYPE GTY((skip)) data[SIZE]; \
    int (*GTY((user)) compare)(TYPE[SIZE], TYPE[SIZE]); \
  }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(double, 5);

/* Test 9: Attribute lists within GTY annotations */
struct GTY((chain_next("next"), chain_prev("prev"),
           user("my_user_function"))) linked_node {
  struct linked_node *GTY((skip)) next;
  struct linked_node *GTY((skip)) prev;
  void (*GTY((user)) callback)(struct linked_node *GTY((skip)) self,
                               int data[]);
  char name[50];
};

/* Test 10: Deeply nested all delimiters */
struct GTY(()) stress_test {
  int (*(*(*GTY((user)) deep_array[2])[3])[4])(int (*(*)(char[5]))[10]);
  
  struct GTY(()) {
    union {
      struct {
        int x;
        int y;
      } GTY((user)) point;
      long coordinates[2];
    } GTY((tag("0"))) data;
    
    void (*GTY((user)) operations[3])(struct stress_test *GTY((skip)),
                                      int params[]);
  } GTY((user)) anonymous;
};

/* Main function to make file compilable */
int main(void) {
  return 0;
}
