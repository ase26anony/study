/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10]), 
                    struct GTY(()) inner_struct { int x; });

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) array_test {
  int arr[(10 + sizeof(struct GTY(()) size_calc { char data[20]; }))];
  void (*func_array[5])(int[][10]);
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY(()) outer_struct {
  /* Nested union with function pointers */
  union GTY(()) nested_union {
    int (*fp1)(char *GTY((length("len"))) str, 
               int len);
    struct GTY(()) {
      int matrix[3][(2 * sizeof(int))];
      void (*callback)(void);
    } data;
  } u;
  
  /* Member with multiple bracket levels */
  int (*multi_array[2][3])(float params[4][5][6]);
  
  /* Initializer-like nested braces in bitfield */
  struct GTY(()) {
    unsigned int flags : 3;
    unsigned int : 5; /* Padding with bitfield */
    struct GTY(()) {
      char *name;
      int values[10];
    } nested;
  } config;
};

/* Test 4: Template-like macro expansion with delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
  struct GTY(()) vector_##TYPE { \
    TYPE data[SIZE]; \
    int (*compare)(TYPE a, TYPE b); \
  }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(struct GTY(()) { int x; double y; }, 5);

/* Test 5: Multiple balanced delimiters in single declaration */
typedef void (*(*GTY((user)) complex_type)[5])(int (*)(char[][20]), 
                                               struct GTY(()) { 
                                                 union GTY(()) { 
                                                   int i; 
                                                   float f; 
                                                 } u; 
                                               });

/* Test 6: Structure with initializer (triggers brace handling) */
static struct GTY(()) initialized_struct = {
  .u = { 
    .data = { 
      .matrix = { {1, 2}, {3, 4}, {5, 6} },
      .callback = NULL
    }
  },
  .multi_array = { NULL, NULL, NULL, NULL, NULL, NULL },
  .config = {
    .flags = 7,
    .nested = {
      .name = "test",
      .values = { [0] = 100, [9] = 200 }
    }
  }
};

/* Test 7: Function with complex return type and parameters */
struct GTY(()) (*get_complex_func(int index))(
  struct GTY(()) param1,
  int (*handler)(struct GTY(()) *ptr, int arr[10][20]))
{
  static struct GTY(()) func_table[] = {
    { .u.data.callback = NULL },
    { .u.data.callback = NULL }
  };
  
  if (index >= 0 && index < 2) {
    return func_table[index].u.data.callback;
  }
  return NULL;
}

/* Test 8: Nested attribute lists in GTY annotations */
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
  struct linked_node *GTY((skip)) next;
  struct linked_node *GTY((skip)) prev;
  void (*GTY((user)) operations[3])(
    struct linked_node *GTY((tag("0"))) node,
    int params[(sizeof(struct linked_node) + 15) / 16]
  );
};

/* Test 9: Union with anonymous struct containing arrays */
union GTY(()) complex_union {
  struct GTY(()) {
    int (*vtable[5])(void);
    char buffer[100];
  } s;
  long (*array_of_funcs[10])(short matrix[3][4]);
};

/* Test 10: Typedef with deeply nested delimiters */
typedef int (*(*(*GTY(()) ultra_complex)[10][20])(float (*)(double))[30])(
  struct GTY(()) {
    int count;
    char *GTY((length("count"))) items[];
  }
);

/* Main function to make the file compilable */
int main(void) {
  /* Access structures to avoid compiler warnings */
  struct array_test at = {0};
  struct outer_struct os = {0};
  struct linked_node ln = {0};
  
  (void)at;
  (void)os;
  (void)ln;
  
  return 0;
}
