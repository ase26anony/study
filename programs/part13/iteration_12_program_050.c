/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10]), 
                    struct GTY(()) { int x; } param);

/* Test 2: Array declarations with complex size expressions */
struct GTY(()) ArrayTest {
  int arr1[(10 + sizeof(struct GTY(()) { char c; int i; }))];
  char arr2[5][(sizeof(int) * 2)];
  void (*arr3[3])(int[][10]);
};

/* Test 3: Deeply nested structure with multiple delimiter types */
struct GTY(()) OuterStruct {
  /* Nested anonymous struct with function pointer array */
  struct GTY(()) {
    int (*callbacks[5])(struct GTY(()) { 
      int x; 
      char y[(20 + sizeof(double))]; 
    } param);
    union GTY(()) {
      int i;
      struct GTY(()) {
        float f;
        double d;
      } s;
    } u;
  } inner;
  
  /* Complex member with all delimiter types */
  void (*(*complex_member))(int (*)(char[][5]), 
                           struct GTY(()) { 
                             union GTY(()) { 
                               int a; 
                               long b; 
                             } u; 
                           });
};

/* Test 4: Multiple balanced delimiters in single declaration */
struct GTY(()) MultiDelimiterTest {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*fn_array[5])(int[][10]);
  
  /* Nested initializer-like structure */
  struct GTY(()) {
    int values[3][2];
    struct GTY(()) {
      char *name;
      int id;
    } entries[4];
  } data;
};

/* Test 5: Typedef with complex nested types */
typedef struct GTY(()) {
  int (*compare)(const void *, const void *);
  void (*GTY((chain_next)) destroy)(void *);
  struct GTY(()) Node {
    void *data;
    struct Node *GTY((skip)) next;
    struct Node *prev;
  } *head;
} ComplexContainer;

/* Test 6: Union with nested structures and arrays */
union GTY(()) NestedUnion {
  struct GTY(()) {
    int type;
    union GTY(()) {
      int int_val;
      double dbl_val;
      struct GTY(()) {
        char *str;
        int len;
      } str_val;
    } value;
  } tagged;
  
  struct GTY(()) {
    int arr[3][(sizeof(void *) + 2)];
    void (*funcs[2])(int, ...);
  } collection;
};

/* Test 7: Structure with bit-fields and nested delimiters */
struct GTY(()) BitFieldTest {
  unsigned int flags : 3;
  signed int value : 10;
  
  /* Pointer to function returning pointer to array */
  int (*(*get_matrix)(void))[10][20];
  
  struct GTY(()) {
    char name[50];
    int (*validator)(const char[], int);
  } metadata;
};

/* Test 8: Recursive structure with function pointers */
struct GTY(()) TreeNode {
  void *GTY((skip)) data;
  struct TreeNode *GTY((chain_next)) children[4];
  int (*traverse)(struct TreeNode *, 
                  void (*visit)(void *, int),
                  int depth);
};

/* Test 9: Template-like pattern simulation using nested types */
#define DECLARE_CONTAINER(TYPE) \
  struct GTY(()) Container_##TYPE { \
    TYPE *items; \
    int (*compare)(TYPE, TYPE); \
    void (*cleanup)(TYPE *); \
  }

DECLARE_CONTAINER(int);
DECLARE_CONTAINER(double);
DECLARE_CONTAINER(struct GTY(()) { int x; char y; });

/* Test 10: Complex initializer (static variable) */
static struct GTY(()) GlobalData = {
  .inner = {
    .callbacks = { NULL, NULL, NULL, NULL, NULL },
    .u = { .s = { .f = 1.0, .d = 2.0 } }
  },
  .complex_member = NULL,
  .data = {
    .values = { {1, 2}, {3, 4}, {5, 6} },
    .entries = {
      { "first", 1 },
      { "second", 2 },
      { "third", 3 },
      { "fourth", 4 }
    }
  }
};

/* Test 11: Multiple GTY annotations in nested contexts */
struct GTY(()) Outer {
  struct GTY((chain_next)) Inner1 {
    int val;
    struct Inner1 *next;
  } *list;
  
  union GTY(()) Inner2 {
    struct GTY(()) {
      int x;
      int y;
    } point;
    struct GTY(()) {
      int start;
      int end;
    } range;
  } data;
  
  void (*GTY((callback)) processor)(struct Outer *, 
                                   union Inner2);
};

/* Test 12: Array of function pointers with complex signatures */
typedef int (*GTY(())
  ComplexFuncArray[3])(struct GTY(()) { 
    int count; 
    char *names[]; 
  } config,
  void (*)(int, ...),
  ...);

/* Test 13: Structure with all delimiter types mixed */
struct GTY(()) AllDelimiters {
  int (*func_ptr)(void);                    /* () */
  int array[10];                           /* [] */
  struct GTY(()) { int x; } anonymous;     /* {} */
  int (*(*nested)[5])(char *);             /* (* [5] ) ( * ) */
  struct GTY(()) {
    union GTY(()) {
      int i;
      long l;
    } u[2];
  } inner;
};

/* Test 14: Deeply nested parentheses and brackets */
struct GTY(()) DeepNest {
  int (*(*(*deep_func)(int (*(*)(char[][3]))[2]))[4])(void);
  
  struct GTY(()) {
    char (*(*strings)[10])[20];
    void (*actions[3][2])(int, struct GTY(()) { 
      int id; 
    });
  } nested;
};

/* Test 15: Multiple attribute lists in GTY annotations */
struct GTY((chain_next, chain_prev, skip)) ListNode {
  void *data;
  struct ListNode *next;
  struct ListNode *prev;
  
  void (*GTY((callback)) on_delete)(struct ListNode *,
                                   void (*)(void *));
};

/* Main function to make the file compilable */
int main(void) {
  /* Access some variables to prevent optimization */
  struct ArrayTest at = {0};
  struct OuterStruct os = {0};
  struct MultiDelimiterTest mdt = {0};
  
  return 0;
}
