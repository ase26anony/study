/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10][20]), 
                    struct GTY(()) inner_struct { int x; double y; });

/* Test 2: Array declarations with complex size expressions */
struct GTY(()) array_test {
  int arr1[(10 + sizeof(struct GTY(()) size_struct { int a; char b; }))];
  char arr2[5][(2 * sizeof(int))];
  void (*arr3[3])(int[][10]);
};

/* Test 3: Deeply nested structure with multiple delimiter types */
struct GTY(()) outer_struct {
  /* Nested anonymous union with braces */
  union GTY(()) {
    struct GTY(()) {
      int x;
      /* Function pointer array member */
      void (*func_array[5])(int (*)(char[][(sizeof(int) + 2)]));
    } s;
    
    /* Another nested struct with initializer-like designators */
    struct GTY(()) {
      double d;
      /* Multi-dimensional array with complex indices */
      float matrix[3][(4 + 1)][2];
    } t;
  } u;
  
  /* Pointer to function returning pointer to array */
  int (*(*complex_ptr)(void))[(10)];
  
  /* Nested GTY annotation on member */
  struct GTY((chain_next)) linked_node * GTY((skip)) next;
};

/* Test 4: Multiple balanced delimiters in single declaration */
union GTY(()) delimiter_test {
  /* Combination of all three delimiter types */
  void (*(*fn_member)(int param[(sizeof(struct {int a;}) / 2)]))
       (char buffer[][(10 + 5)]);
  
  /* Complex array of function pointers */
  int (*array_of_funcs[3])(float, double, 
                           struct GTY(()) { short s; long l; });
};

/* Test 5: Template-like pattern simulation using nested types */
#define DECLARE_VECTOR(T) \
  struct GTY(()) vector_##T { \
    T* GTY((length("len"))) data; \
    size_t len; \
    void (*resize)(struct vector_##T* self, size_t new_len); \
  }

DECLARE_VECTOR(int);
DECLARE_VECTOR(double);
DECLARE_VECTOR(struct GTY(()) point { int x; int y; });

/* Test 6: Structure with complex initializer (simulated) */
struct GTY(()) with_initializer {
  int values[5];
  struct GTY(()) nested {
    char* name;
    int id;
  } obj;
};

/* Simulated initializer that would be parsed */
static struct with_initializer GTY(()) init_example = {
  .values = {[(2 + 1)] = 42, [0] = 1, [4] = 99},
  .obj = {.name = "test", .id = 100}
};

/* Test 7: Recursive structure with function pointers */
typedef struct GTY(()) tree_node {
  int value;
  /* Function pointer with nested parameter */
  void (*visitor)(struct tree_node* node, 
                  void (*callback)(int result[(10)]));
  /* Array of child nodes */
  struct tree_node* GTY((length("child_count"))) children;
  int child_count;
  /* Nested union for different node types */
  union GTY(()) {
    struct GTY(()) {
      char* str_data;
      int str_len;
    } string_node;
    struct GTY(()) {
      double* GTY((length("array_len"))) array_data;
      int array_len;
    } array_node;
  } data;
} tree_node_t;

/* Test 8: Multiple levels of nested GTY annotations */
struct GTY(()) level1 {
  struct GTY((chain_next)) level2 {
    struct GTY((chain_prev)) level3 {
      /* Function pointer with attributes in parameters */
      int (*processor)(char input[(20)], 
                       struct GTY(()) config { int mode; }* cfg);
      /* Multi-dimensional array with size expression */
      unsigned char bitmap[(16)][(32)];
    }* l3;
    int count;
  }* l2;
  float value;
};

/* Test 9: Complex typedef with all delimiter types */
typedef union GTY(()) {
  struct GTY(()) {
    /* Array of function pointers returning pointers to arrays */
    int (*(*callbacks[5])(void))[(10 + sizeof(int))];
    /* Nested structure with bitfield */
    struct GTY(()) {
      unsigned int flags : 3;
      unsigned int : 5; /* padding */
      unsigned int mode : 4;
    } status;
  } data;
  
  /* Anonymous struct with initializer-like members */
  struct GTY(()) {
    double matrix[2][(3)][4];
    void (*operations[2])(double[][(3)][4]);
  } ops;
} complex_union_t;

/* Test 10: Edge case - empty balanced delimiters */
struct GTY(()) empty_delimiters {
  int empty_array[0];  /* Empty array */
  void (*null_func)(void);  /* Function taking no parameters */
  struct GTY(()) empty_struct { };  /* Empty structure */
  union GTY(()) { } empty_union;  /* Empty union */
};

/* Main function to make the file compilable */
int main(void) {
  /* These structures would normally be used by GCC's garbage collector */
  /* For testing purposes, we just return */
  return 0;
}
