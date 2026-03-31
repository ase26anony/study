/* complex_gty_test.c - Test file to exercise delimiter handling in gengtype-parse.cc */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10]), 
                    struct GTY(()) inner_struct { int x; });

/* Test 2: Array with complex dimension expression */
struct GTY(()) array_test {
  int arr[(10 + sizeof(struct GTY(()) size_struct { char c; }))];
  int (*GTY((chain_next)) ptr_arr[5])(int[][10]);
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY(()) outer_struct {
  /* Nested anonymous union with braces */
  union GTY(()) {
    struct GTY(()) {
      int x;
      char y[20];
    } s;
    
    /* Function pointer array in union */
    void (*GTY(()) func_array[3])(int, char);
  } u;
  
  /* Complex function pointer member */
  int (*GTY((chain_next, chain_prev))
    complex_member)(struct GTY(()) param_struct { 
      int a; 
      double b[5]; 
    }, ...);
};

/* Test 4: Multiple balanced delimiters in single declaration */
typedef void (*(*GTY(()) nested_fp_array[5])(int, float))[10];

/* Test 5: Structure with initializer-like nested braces */
struct GTY(()) init_test {
  struct GTY(()) {
    int values[3];
    struct GTY(()) {
      char *GTY((length("len"))) name;
      int len;
    } info;
  } nested;
  
  /* Array of function pointers with attributes */
  void (*GTY((callback)) handlers[4])(struct GTY(()) event { 
    int type; 
    union GTY(()) {
      int i;
      void *GTY((skip)) p;
    } data;
  });
};

/* Test 6: Template-like pattern using nested parentheses */
#define DECLARE_VECTOR(T) \
  struct GTY(()) vector_##T { \
    T *GTY((length("size"))) data; \
    int size; \
    int (*GTY(()) compare)(const T*, const T*); \
  }

DECLARE_VECTOR(int);
DECLARE_VECTOR(struct GTY(()) point { int x; int y; });

/* Test 7: Multiple levels of nested delimiters */
struct GTY(()) ultimate_test {
  /* Combination of all three delimiters */
  int (*(*(*GTY(()) crazy)[5])(int (*)(char[][10]), 
                               struct { 
                                 union { 
                                   int x; 
                                   double y; 
                                 } u; 
                               }))[3];
  
  /* Nested GTY annotations */
  struct GTY((for_user)) user_type {
    struct GTY((chain_next)) chain {
      struct chain *GTY((skip)) next;
      void *GTY((skip)) data;
    } *list;
    
    /* Array with designators in initializer position */
    int matrix[2][3];
  } *user;
};

/* Test 8: Union with bit-fields and arrays */
union GTY(()) bitfield_test {
  struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
    unsigned int value : 8;
  } bits;
  
  char bytes[2];
  
  /* Function pointer in union */
  void (*GTY(()) action)(int, ...);
};

/* Test 9: Recursive structure with function pointer */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((chain_next)) left;
  struct tree_node *GTY((chain_prev)) right;
  
  /* Visitor function pointer */
  void (*GTY(()) visit)(struct tree_node *,
                        void (*)(int, char[]),
                        int depth);
};

/* Test 10: Mixed attributes and nested types */
struct GTY((user)) final_test {
  /* Chain of structures */
  struct GTY((chain_next, chain_prev)) link {
    struct link *GTY((skip)) next;
    struct link *GTY((skip)) prev;
    void *GTY((skip)) payload;
    
    /* Array of callback functions */
    int (*GTY((callback)) callbacks[3])(
      struct link *,
      int (*)(char *, int),
      void *GTY((skip)) context
    );
  } *head;
  
  /* Multi-dimensional array with complex type */
  struct GTY(()) matrix_cell {
    double value;
    int (*GTY(())) validator[2][2](double, double);
  } matrix[10][10];
};

/* Additional test cases to ensure full coverage */

/* Test 11: Empty balanced delimiters */
struct GTY(()) empty_test {
  int empty_array[0];
  void (*empty_func)();
  struct {} empty_struct;
};

/* Test 12: Nested parentheses in macro arguments */
#define DEFINE_CALLBACK(name, ret, params) \
  ret (*GTY(()) name) params

DEFINE_CALLBACK(my_callback, int, (int x, char *GTY((skip)) str));

/* Test 13: Attribute with nested parentheses */
struct GTY((tag("MY_TYPE"), 
           desc("Nested: %1", 
                struct GTY(()) desc_helper { 
                  int id; 
                  char *name; 
                }))) 
attributed_struct {
  int id;
  char *GTY((length("strlen(name)"))) name;
};

/* Test 14: Multiple levels of array brackets */
struct GTY(()) deep_arrays {
  int (*deep1[2])[3][4];
  int (*(*deep2)[5])[6][7];
  int (*(*(*deep3)[8])[9])[10][11];
};

/* Test 15: Mixed delimiters with GTY in the middle */
typedef struct GTY(()) {
  union {
    int x;
    struct GTY(()) {
      char a;
      short b;
    } s;
  } u;
  
  void (*GTY((chain_next)) process)(
    int array[],
    struct { int count; } info
  );
} mixed_delim_type;

/* Dummy main to make file compilable */
int main(void) {
  return 0;
}
