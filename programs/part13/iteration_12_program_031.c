/* complex-gty-test.c - Test file for exercising gengtype-parse.cc delimiter handling */

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
  int arr[(10 + sizeof(struct GTY(()) size_struct { char c; int i; }))];
  void (*func_array[5])(int[][10]);
};

/* Test 3: Deeply nested structures with all delimiter types */
struct GTY(()) outer_struct {
  /* Nested struct with initializer-like designators */
  struct GTY(()) level1 {
    int x;
    union GTY(()) level2 {
      char c;
      struct GTY(()) level3 {
        void (*func)(int (*)(char[5]), struct { int a; double b; });
        int matrix[3][(2 * sizeof(int))];
      } deep;
    } inner;
  } nested;
  
  /* Complex member with multiple delimiters */
  int (*(*complex_member)[(sizeof(int) + 1)])(float (*params)[10]);
};

/* Test 4: Union containing array of function pointers */
union GTY(()) complex_union {
  struct GTY(()) {
    int count;
    void (*handlers[(sizeof(void*) * 2)])(struct GTY(()) param { int x; y; });
  } data;
  
  char buffer[100];
};

/* Test 5: Typedef with nested GTY annotations */
typedef struct GTY(()) base_type {
  GTY((skip)) int skip_me;
  GTY((length("len"))) int *variable_array;
  int len;
} *GTY((tag("1"))) base_type_ptr;

/* Test 6: Structure with bit-fields and nested initializer-like syntax */
struct GTY(()) bitfield_test {
  unsigned int flags : 3;
  unsigned int : 5;  /* unnamed bit-field */
  struct GTY(()) {
    int x : 8;
    int y : 8;
  } packed;
  
  /* Array with designated initializer pattern in comment */
  int values[4]; /* Would be initialized as {[0]=1, [2]=3} */
};

/* Test 7: Multiple levels of pointer indirection with attributes */
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
  struct linked_node * GTY((skip)) next;
  struct linked_node *prev;
  void (* GTY((callback)) operations[3])(struct linked_node *);
  int data[((sizeof(void*) > 4) ? 8 : 4)];
};

/* Test 8: Template-like pattern using nested structures */
struct GTY(()) container {
  struct GTY(()) pair {
    void *first;
    void *second;
  } items[10];
  
  /* Function returning pointer to array */
  struct pair (*(*get_pairs)(int index))[10];
};

/* Test 9: Structure with all delimiter types in one member */
struct GTY(()) delimiter_bomb {
  /* Contains: (), [], {} */
  void (*(*complex_array[((2+3)*4)])[(sizeof(double)/2)])(
    int param1[(1+2)],
    struct { 
      char c; 
      int arr[5]; 
    } param2
  );
};

/* Test 10: Recursive structure with function pointer */
struct GTY(()) tree_node {
  int value;
  struct tree_node * GTY((skip)) left;
  struct tree_node *right;
  int (*compare)(struct tree_node *, struct tree_node *);
  void (*traversal[3])(struct tree_node *nodes[], int count);
};

/* Test 11: Union with anonymous struct containing nested delimiters */
union GTY(()) anonymous_test {
  struct {
    int type;
    union {
      int int_val;
      struct {
        char *str;
        int len;
      } string;
      void (*func)(int, char *);
    } data;
  } variant;
  
  long long raw[2];
};

/* Test 12: Structure with macro-like patterns that expand to delimiters */
#ifdef TEST_MACRO
#  define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#  define CALLBACK_TYPE void (*)(int, char *)
#else
#  define ARRAY_SIZE(x) 10
#  define CALLBACK_TYPE void (*)(void)
#endif

struct GTY(()) macro_test {
  CALLBACK_TYPE callbacks[ARRAY_SIZE(((int[]){1,2,3,4}))];
  int sizes[ARRAY_SIZE(((struct {int a; char b;}[]){ {1,'a'}, {2,'b'} }))];
};

/* Test 13: Nested GTY annotations in typedef */
typedef struct GTY(()) {
  int id;
  struct GTY((chain_next("next"))) {
    char *name;
    struct GTY(()) *next;
  } entries[5];
} GTY((tag("NESTED_TYPEDEF"))) nested_typedef_t;

/* Test 14: Complex initializer pattern (in comment for parser exercise) */
struct GTY(()) with_init {
  int x;
  double y;
  /* Initializer would look like: { .x = 5, .y = 3.14, .z = { [0] = 1, [2] = 3 } } */
  int z[3];
};

/* Test 15: Multiple attribute lists in GTY annotation */
struct GTY((chain_next, chain_prev, length("count"), reorder("reorder_fn"))) 
  attribute_test {
  struct attribute_test *next;
  struct attribute_test *prev;
  int count;
  void ** GTY((length("count"))) items;
  void (*reorder_fn)(struct attribute_test *);
};

/* Main function to make the file compilable (GTY macros expand to nothing normally) */
int main(void) {
  return 0;
}
