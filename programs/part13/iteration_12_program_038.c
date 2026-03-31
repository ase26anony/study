/* complex_gty_test.c - Test file for gengtype delimiter parsing coverage */

/* Include necessary headers for GTY annotations */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* ====== Test Case 1: Complex function pointer with nested parentheses ====== */
/* This will trigger consume_balanced('(', ')') multiple times */

/* A callback type with nested parameter list */
typedef int (*nested_callback_t)(int (*inner)(char *), double);

/* Structure with complex function pointer member */
struct GTY(()) complex_func_struct {
  /* Function pointer returning function pointer */
  void (*(*GTY((tag("1")))) (int, double)) (char *, float);
  
  /* Array of function pointers */
  nested_callback_t GTY((skip)) callbacks[5];
  
  /* Function pointer with attributes in parameters */
  void (*handler) (int __attribute__((unused)), 
                   struct complex_func_struct *GTY((skip)));
};

/* ====== Test Case 2: Array declarations with complex dimensions ====== */
/* This will trigger consume_balanced('[', ']') */

/* Structure with complex array dimensions */
struct GTY(()) array_test {
  /* Array with computed size using parentheses */
  int arr1[(10 + (sizeof(struct complex_func_struct) / 4))];
  
  /* Multi-dimensional array */
  char matrix[5][(2 * 3)][10];
  
  /* Pointer to array with function pointer elements */
  void (*(*func_array)[(3 + 2)])();
  
  /* Nested array in typedef */
  typedef int nested_array_t[5][(sizeof(int) * 2)];
  nested_array_t GTY((skip)) data;
};

/* ====== Test Case 3: Nested structures and unions with braces ====== */
/* This will trigger consume_balanced('{', '}') */

/* Inner anonymous union */
union GTY(()) inner_union {
  int x;
  char *GTY((tag("2"))) str;
  struct {
    float f;
    double d;
  } nested;
};

/* Structure with deeply nested anonymous struct */
struct GTY(()) deeply_nested {
  int id;
  
  /* Anonymous struct member */
  struct {
    /* Struct within struct */
    struct {
      int a;
      union inner_union u;
    } inner;
    
    /* Array within anonymous struct */
    int values[5];
  } container;
  
  /* Union with anonymous struct */
  union {
    struct {
      char *GTY((tag("3"))) name;
      int count;
    } s;
    long value;
  } choice;
};

/* ====== Test Case 4: Combined all delimiters in single declaration ====== */

/* Type definition combining all three delimiter types */
typedef void (*(*combined_type[(2 + 3)])(
    struct deeply_nested *GTY((skip)), 
    int arr[][(5 * 2)]
)) (char *str, 
    struct { 
      int x; 
      char *GTY((tag("4"))) y; 
    } param);

/* Structure using the combined type */
struct GTY(()) master_struct {
  combined_type GTY((skip)) complex_field;
  
  /* Initializer-like designator (triggers braces) */
  struct array_test config;
  
  /* Function pointer with all delimiters */
  int (*(*signal_handler[3])(
      void (*)(int), 
      int list[]
  )) (struct master_struct *GTY((skip)));
};

/* ====== Test Case 5: GTY annotations with complex attribute lists ====== */

/* Chain of structures with nested GTY attributes */
struct GTY((chain_next("next"), chain_prev("prev"))) chain_node {
  int data;
  struct chain_node *GTY((skip)) next;
  struct chain_node *GTY((skip)) prev;
  
  /* Pointer with nested GTY in typedef */
  typedef struct chain_node *(*node_factory_t)(
      int count,
      struct { 
        char *GTY((tag("5"))) name; 
      } config
  );
  node_factory_t GTY((skip)) factory;
};

/* ====== Test Case 6: Template-like patterns using macros ====== */

/* Macro that creates nested structures */
#define DECLARE_NESTED_TYPE(name, type) \
  struct GTY(()) name##_container { \
    type GTY((skip)) value; \
    struct { \
      type GTY((skip)) array[5]; \
      void (*processor)(type *, int); \
    } helper; \
  }

/* Instantiate the macro with complex type */
DECLARE_NESTED_TYPE(my_type, struct deeply_nested *);

/* ====== Test Case 7: Complex initializer (triggers braces) ====== */

/* Global variable with complex initializer */
static struct array_test GTY((skip)) global_array = {
  .arr1 = {1, 2, 3, [(10 + 2) / 4] = 42},
  .matrix = {
    [0] = {"hello"},
    [2] = {{'a', 'b'}, {'c', 'd'}}
  },
  .data = {
    [0] = {1, 2, 3, 4, 5},
    [2] = {[3] = 99, [1] = 77}
  }
};

/* ====== Test Case 8: Even more nesting ====== */

/* Ultimate nesting test */
struct GTY(()) ultimate_nest {
  /* Function returning pointer to array of function pointers */
  void (*(*(*level1)(int))(float))[5];
  
  /* Nested anonymous struct with all delimiters */
  struct {
    int (*(*func_ptr_array[3])(
        char *argv[],
        void (*callback)(
            struct { 
              int x; 
              int y[2]; 
            } point
        )
    ))();
    
    /* Union with anonymous struct containing array */
    union {
      struct {
        int matrix[2][(3 + 1)];
        void (*handlers[2])(
            int param,
            struct ultimate_nest *self
        );
      } s;
      long raw[4];
    } u;
  } inner;
};

/* ====== Test Case 9: Typedef chains with delimiters ====== */

/* Chain of typedefs with increasing complexity */
typedef int simple_t;
typedef simple_t *pointer_t;
typedef pointer_t (*function_t)(int, char *);
typedef function_t array_t[5];
typedef struct { array_t funcs; } wrapper_t;
typedef wrapper_t *(*factory_t)(int size, wrapper_t init);

/* Structure using the complex typedef */
struct GTY(()) typedef_user {
  factory_t GTY((skip)) create;
  wrapper_t GTY((skip)) instance;
};

/* ====== Test Case 10: Bit-fields and packed structures ====== */

struct GTY(()) bitfield_test {
  unsigned int flag:1;
  unsigned int count:5;
  
  /* Anonymous struct with bitfields */
  struct {
    unsigned int a:2;
    unsigned int b:3;
    unsigned int :0;  /* Force alignment */
    unsigned int c:4;
  } parts;
  
  /* Array of bitfield-like structures */
  struct {
    char start;
    char end;
  } ranges[3];
};

/* Main function to make the file compilable */
int main() {
  return 0;
}
