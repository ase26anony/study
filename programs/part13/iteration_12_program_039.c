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
  int arr[(10 + sizeof(struct GTY(()) size_struct { char c; }))];
  char * GTY((length("arr[0]"))) ptr_field;
};

/* Test 3: Deeply nested structures with all delimiter types */
struct GTY(()) outer_struct {
  /* Case '(': function pointer with complex signature */
  void (* GTY((skip)) fp1)(int (*(*nested_fp)[5])(char *), 
                           struct { int a; double b; });
  
  /* Case '[': multi-dimensional array with nested brackets */
  int matrix[3][(sizeof(int) * 2)][10];
  
  /* Case '{': nested anonymous struct with initializer-like syntax */
  struct GTY(()) {
    union GTY(()) {
      int x;
      long y;
    } u;
    struct GTY(()) inner {
      short s;
      char c;
    } inner_struct;
  } anonymous;
};

/* Test 4: Template-like pattern using macros with nested delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
  struct GTY(()) vector_##TYPE { \
    TYPE data[SIZE]; \
    int (* GTY((callback)) compare)(TYPE a, TYPE b); \
  }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(struct GTY(()) { int x; char y; }, 5);

/* Test 5: Union with complex nested types */
union GTY(()) complex_union {
  /* Function pointer array */
  void (*func_array[3])(int[][10], struct GTY(()) { int tag; });
  
  /* Nested struct with bitfields */
  struct GTY(()) {
    unsigned int flag:1;
    unsigned int value:31;
    int (* GTY((chain_next)) next)(void);
  } bits;
  
  /* Anonymous union inside union */
  union GTY(()) {
    double d;
    float f[4];
  } numbers;
};

/* Test 6: Type definition with all three delimiters in sequence */
typedef struct GTY(()) {
  /* Combination: (* []) () [] {} */
  void (*(*complex_array[5])[3])(int param[10], 
                                 struct GTY(()) { 
                                   union GTY(()) { 
                                     int i; 
                                     char c; 
                                   } u; 
                                 }) GTY((tag("complex")));
  
  /* Initializer-like nested braces */
  struct GTY(()) config {
    int values[5];
    struct GTY(()) {
      char *name;
      int id;
    } entries[3];
  } settings;
} ultimate_type;

/* Test 7: Recursive structure with function pointers */
struct GTY(()) tree_node {
  int value;
  struct GTY((chain_next("next"), chain_prev("prev"))) tree_node *next;
  struct GTY((chain_next("next"), chain_prev("prev"))) tree_node *prev;
  int (* GTY((callback)) process)(struct tree_node *n, 
                                  int (*(*callback_array)[5])(void),
                                  char data[][20]);
};

/* Test 8: Multiple GTY annotations with nested attribute lists */
struct GTY((user("my_type"), desc("nested_test"))) annotated_struct {
  /* GTY annotation with nested parentheses in arguments */
  char * GTY((length("strlen(data) + 1"),
              reorder("data", "size"))) data;
  
  int size;
  
  /* Function pointer with GTY annotation on parameters */
  void (* GTY((skip("skip_this"))) 
    processor)(struct GTY((user("param_type"))) { 
                 int count; 
                 char * GTY((length("count"))) buffer; 
               } *param);
};

/* Test 9: Array of function pointers returning struct pointers */
struct GTY(()) result {
  int status;
  char message[100];
};

typedef struct result* (* GTY((chain_next)) 
  result_generator[5])(int input, 
                       void (* GTY((callback)) progress)(int),
                       char options[][50]);

/* Test 10: Deeply nested initializer-like constructs */
static struct GTY(()) deeply_nested = {
  .fp = (void (*)(int (*)(char[10]), struct { int x; }))0,
  .matrix = { [0] = { [0] = { 1, 2, 3 } },
              [1] = { [2] = { 4, 5, 6 } } },
  .anonymous = { .u = { .x = 42 },
                 .inner_struct = { .s = 10, .c = 'A' } },
  .nested = { { { .i = 100 }, { .c = 'Z' } },
              { { .i = 200 }, { .c = 'Y' } } }
};

/* Test 11: Macro expansion with nested delimiters */
#define CREATE_NESTED(TYPE, NAME, SIZE) \
  TYPE NAME[SIZE] = { \
    [0] = { .x = 1, .y = { .a = 2, .b = { .c = 3 } } }, \
    [SIZE-1] = { .x = 99, .y = { .a = 88, .b = { .c = 77 } } } \
  }

struct GTY(()) level3 { int c; };
struct GTY(()) level2 { int a; struct level3 b; };
struct GTY(()) level1 { int x; struct level2 y; };

CREATE_NESTED(struct level1, nested_array, 10);

/* Test 12: Complex typedef with all delimiters */
typedef int (*(*(* GTY((user("complex_typedef")))
  complex_typedef_func)(int (*(*arg1)[5])(char *), 
                        struct GTY(()) { int id; }))
  [10])(float, double[3][3]);

/* Main function to make the file compilable */
int main(void) {
  /* Reference all types to avoid unused warnings */
  struct array_test at = {0};
  struct outer_struct os = {0};
  struct complex_union cu = {0};
  ultimate_type ut = {0};
  struct tree_node tn = {0};
  struct annotated_struct as = {0};
  
  (void)at;
  (void)os;
  (void)cu;
  (void)ut;
  (void)tn;
  (void)as;
  
  return 0;
}
