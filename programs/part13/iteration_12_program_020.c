/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef int (*GTY((chain_next)) complex_callback_t)(char buffer[256], 
                                                    void (*nested)(int));

struct GTY(()) outer_struct {
  /* Nested parentheses in function pointer */
  void (* GTY((tag("1"))) func_ptr)(int (*inner_cb)(char[10][20]), 
                                    struct { int x; } GTY(()) nested);
  
  /* Array with complex dimension expression */
  int GTY((skip)) arr[(10 + sizeof(struct GTY(()) dummy))];
  
  /* Chain of pointers with attributes */
  struct outer_struct * GTY((chain_next, chain_prev)) next;
  
  /* Nested structure with its own GTY annotation */
  struct GTY(()) inner {
    /* Array of function pointers */
    complex_callback_t (*callbacks[5])(int[][10]);
    
    /* Bit-field with initializer */
    unsigned int flags : 8;
    
    /* Another nested structure */
    union GTY(()) deep_union {
      int ival;
      /* Pointer array with nested parentheses */
      char *(* GTY((length("strlen(%h)"))) str_array[3])(void);
    } u;
  } inner_struct;
};

/* Test 2: Multiple balanced delimiter types in single declaration */
typedef void (* GTY(()) complex_fn_array_t[3])(
    int param1,
    char param2[][(sizeof(int) * 2)],
    struct GTY(()) { 
      double d; 
      int i[5]; 
    } param3
);

/* Test 3: Deeply nested braces in initializer */
static struct outer_struct GTY(()) global_var = {
  .func_ptr = NULL,
  .arr = {0, 1, 2, [(10 + sizeof(int)) - 1] = 42},
  .next = NULL,
  .inner_struct = {
    .callbacks = {NULL, NULL, NULL, NULL, NULL},
    .flags = 0xAA,
    .u = {
      .str_array = {NULL, NULL, NULL}
    }
  }
};

/* Test 4: Template-like macro with nested delimiters */
#define DECLARE_GTY_ARRAY(TYPE, SIZE) \
  TYPE GTY(()) gty_array_##TYPE[SIZE]

DECLARE_GTY_ARRAY(struct { 
  int x; 
  char * GTY((length("strlen(%h)"))) str; 
}, 10);

/* Test 5: Union with function pointer containing array parameter */
union GTY(()) complex_union {
  int (*processor)(char input[256][128], 
                   void (*callback)(struct { int a; double b; } GTY(())));
  
  struct GTY(()) {
    /* Multi-dimensional array with complex indices */
    float matrix[3][(2 * sizeof(double))];
    
    /* Pointer to array of pointers */
    int *(*(*nested_ptr)[5])[10];
  } data;
};

/* Test 6: Typedef with all delimiter types */
typedef struct GTY(()) {
  /* Parentheses in function type */
  __typeof__(int (*)(char[5])) func_type;
  
  /* Brackets in array declaration */
  int nested_array[3][2][1];
  
  /* Braces in anonymous struct member */
  struct { 
    union { 
      short s; 
      long l; 
    } u; 
  } anonymous;
} all_delimiters_t;

/* Test 7: Recursive structure with complex attributes */
struct GTY((chain_next, chain_prev)) recursive_node {
  char * GTY((length("strlen(%h)"))) name;
  
  /* Array of pointers to functions returning pointers to arrays */
  int (*(*(*handlers[3])(void))[5])(char);
  
  /* Nested structure with initializer */
  struct GTY(()) {
    int counters[4];
    /* Anonymous union with bit-fields */
    union {
      unsigned int packed : 16;
      struct { unsigned char a : 4, b : 4, c : 8; } parts;
    } flags;
  } state;
  
  struct recursive_node *next;
  struct recursive_node *prev;
};

/* Test 8: Multiple levels of nested parentheses */
typedef int (*(*(*deep_nested_func)(int (*(*)(double))[3]))(char))(void);

/* Test 9: Structure with designated initializers containing arrays */
static struct GTY(()) init_test {
  int values[5];
  struct GTY(()) nested {
    char *strings[2];
    float matrix[2][2];
  } inner;
} init_var = {
  .values = {[0] = 1, [4] = 5},
  .inner = {
    .strings = {"test", "array"},
    .matrix = {{1.0, 2.0}, {3.0, 4.0}}
  }
};

/* Test 10: Edge case - empty balanced delimiters */
struct GTY(()) empty_delimiters {
  void (*empty_func)(void);
  int empty_array[0];
  struct {} empty_struct;
};

/* Main function to make the file compilable */
int main(void) {
  /* Access variables to prevent optimization */
  global_var.arr[0] = 1;
  init_var.values[0] = 2;
  return 0;
}
