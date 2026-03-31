/* complex-gty-test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10]), 
                    struct GTY(()) { int x; double y; } param);

/* Test 2: Array with complex dimension expression */
struct GTY(()) ArrayTest {
  int arr1[(10 + sizeof(struct GTY(()) { int a; char b; }))];
  char arr2[5][(sizeof(int) * 2)];
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY(()) OuterStruct {
  /* Function pointer array */
  void (*GTY((skip)) fp_array[3])(int[][10]);
  
  /* Nested anonymous union with bitfields */
  union GTY(()) {
    struct GTY(()) {
      unsigned int field1:4;
      unsigned int field2:((sizeof(int) * 8) - 4);
    } bits;
    long long full;
  } data;
  
  /* Pointer to function returning pointer to array */
  int (*(*GTY((user)) complex_ret)())[10];
};

/* Test 4: Multiple balanced delimiters in single declaration */
typedef int (*(*GTY((desc("test"))) 
  nested_fp)(int (*)(char[5]), void (*[2])()))[3][4];

/* Test 5: Structure with initializer-style nested braces */
struct GTY(()) InitStyle {
  struct GTY(()) {
    int x;
    struct GTY(()) {
      char c;
      double d;
    } inner;
  } nested;
  
  /* Array with designated initializer pattern in comment */
  int matrix[2][3]; /* Would be initialized as {{1,2,3},{4,5,6}} */
};

/* Test 6: Template-like pattern using nested types */
#define DECLARE_VECTOR(T) \
  struct GTY(()) vector_##T { \
    T* GTY((length("len"))) data; \
    size_t len; \
    void (*GTY((skip)) sort)(T* GTY((skip)) arr[], int (*cmp)(const T*, const T*)); \
  }

DECLARE_VECTOR(int);
DECLARE_VECTOR(struct GTY(()) { int x; char* GTY((skip)) name; });

/* Test 7: Union containing all delimiter types */
union GTY(()) AllDelimiters {
  int (*func_ptr)(int[5], struct GTY(()) { int a; } param);
  char multi_array[2][(10 + 5)][sizeof(double)];
  struct GTY(()) {
    void (*nested[2])(void);
    union GTY(()) {
      int i;
      char c[4];
    } u;
  } s;
};

/* Test 8: Recursive structure with function pointers */
struct GTY(()) TreeNode {
  char* GTY((tag("0"))) data;
  struct TreeNode* GTY((left, right)) children[2];
  int (*GTY((skip)) compare)(struct TreeNode*, struct TreeNode*);
  void (*GTY((skip)) traverse)(struct TreeNode*, 
                               void (*)(char* GTY((skip)), int[3]));
};

/* Test 9: Complex attribute list in GTY */
struct GTY((chain_next("next"), chain_prev("prev"),
           desc("%0.print()"), skip, user))
  LinkedListNode {
  struct LinkedListNode* GTY((skip)) next;
  struct LinkedListNode* GTY((skip)) prev;
  void* GTY((skip)) data;
  void (*GTY((skip)) print)(struct LinkedListNode*,
                           int format[2][(sizeof(void*) + 1)]);
};

/* Test 10: Multiple levels of nested parentheses and brackets */
typedef void (*(*GTY((desc("ultimate_test")))
  ultimate_fp_type[2])(int, 
                      struct GTY(()) { 
                        int (*nested[3])(char[(10 * 2)]); 
                      }))
  (int (*)(int[][10][20]), 
   union GTY(()) { 
     long l; 
     double d[(5 + (3 * 2))]; 
   });

/* Main structure that uses many of the above types */
struct GTY(()) MainTestStruct {
  complex_func_ptr func1;
  struct ArrayTest arrays;
  struct OuterStruct outer;
  nested_fp fp_test;
  struct InitStyle init;
  struct vector_int int_vec;
  struct vector_struct_GTY____int_x_char_name name_vec;
  union AllDelimiters delims;
  struct TreeNode* root;
  struct LinkedListNode* list_head;
  ultimate_fp_type ultimate;
  
  /* Direct nested initializer-like pattern */
  struct GTY(()) {
    int a;
    struct GTY(()) {
      int b[3];
      char c;
    } inner;
  } direct_nested;
};

/* Additional test: Typedef with complex declarator */
typedef struct GTY(()) {
  int x;
  void (*methods[3])(int, int);
} Interface, *GTY((skip)) InterfacePtr;

/* Test function pointer as struct member with attributes */
struct GTY(()) CallbackTest {
  void (*GTY((callback, skip)) callback_fn)(int, 
                                           struct GTY(()) { 
                                             int result[2]; 
                                             char status; 
                                           });
  int (*GTY((skip)) validate)(char* GTY((length("len"))) str, 
                             int len,
                             int (*)(const char*, const char*));
};

/* Final comprehensive test combining everything */
struct GTY((user)) ComprehensiveTest {
  /* Multi-dimensional function pointer array */
  int (*(*GTY((skip)) complex_array[2][3])(void))[4];
  
  /* Nested anonymous struct with bitfields and arrays */
  struct GTY(()) {
    unsigned int flags:((sizeof(int)*8) - 2);
    unsigned int:2; /* unnamed bitfield */
    char name[(20 + 1)];
  } info;
  
  /* Union containing function pointer with nested params */
  union GTY(()) {
    int (*func)(int (*)(int), char[10][20]);
    struct GTY(()) {
      double matrix[2][(3 * 4)];
      void (*ops[2])(struct GTY(()) { int x; } param);
    } s;
  } u;
  
  /* Pointer to self-referential structure */
  struct ComprehensiveTest* GTY((skip)) next;
};

/* Make sure the file has some actual code for compilation */
#ifdef __GNUC__
void dummy_function(void) {
  /* Empty but prevents "ISO C forbids an empty translation unit" warning */
}
#endif
