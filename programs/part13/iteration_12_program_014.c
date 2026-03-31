/* complex_gty_test.c - Test file for exercising gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10]), 
                    struct GTY(()) { int x; double y; } param);

struct GTY(()) outer_struct_1 {
  complex_func_ptr func_field;
  int data;
};

/* Test 2: Array declarations with complex sizes containing parentheses */
struct GTY(()) array_test {
  int simple_arr[10];
  char* GTY((length("(sizeof(struct outer_struct_1) + 100)"))) 
    dynamic_arr;
  double nested_arr[(10 + sizeof(struct outer_struct_1))];
  void* ptr_arr[((5 * 2) + 3)];
};

/* Test 3: Deeply nested structures with multiple delimiter types */
union GTY(()) deep_nested {
  struct GTY(()) level1 {
    struct GTY(()) level2 {
      struct GTY(()) level3 {
        int (*func_array[5])(int[][10], 
                             struct { 
                               int a; 
                               char b[20]; 
                             });
        union GTY(()) {
          short s;
          long l;
        } inner_union;
      } l3;
      float matrix[3][(2+3)];
    } l2;
    char* GTY((tag("0"))) string_ptr;
  } l1;
  double dbl;
};

/* Test 4: Multiple balanced delimiter types in single declaration */
struct GTY(()) complex_member {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*fn_array[5])(int[][10], 
                      struct GTY(()) { 
                        int x; 
                        int y[(2+3)*4]; 
                      });
  
  /* Nested initializer-like syntax in array dimension */
  int computed_size[({ int x = 5; int y = 10; x + y; })];
};

/* Test 5: Structure with attribute lists containing nested parentheses */
struct GTY((chain_next("next"), 
            chain_prev("prev"),
            user("my_tag"))) linked_node {
  struct linked_node *GTY((skip)) next;
  struct linked_node *GTY((skip("prev"))) prev;
  int data;
  
  /* Member with function pointer returning pointer to array */
  int (*(*complex_member)(void))[10];
};

/* Test 6: Typedef with deeply nested parentheses */
typedef int (*(*(*GTY(()) nested_fp_type)(int, 
                                          char*))(float))(
                                            double, 
                                            struct GTY(()) { 
                                              int tag; 
                                              void* data; 
                                            });

/* Test 7: Union containing anonymous struct with bitfields and arrays */
union GTY(()) bitfield_test {
  struct GTY(()) {
    unsigned int flag:1;
    unsigned int count:7;
    unsigned int values[8];
    char name[(16 + sizeof(int))];
  } bits;
  unsigned long long raw;
};

/* Test 8: Structure with nested initializer-style members */
struct GTY(()) with_initializers {
  int x;
  struct GTY(()) inner {
    int a;
    char b;
  } inner_struct;
  
  /* Simulate designated initializers in type context */
  int array_with_designator[10];
};

/* Test 9: Template-like pattern using nested types */
#define DECLARE_GTY_CONTAINER(TYPE) \
  struct GTY(()) container_##TYPE { \
    TYPE* GTY((length("size"))) items; \
    int size; \
    int (*(*get_func)(struct container_##TYPE*))(); \
  }

DECLARE_GTY_CONTAINER(int);
DECLARE_GTY_CONTAINER(double);
DECLARE_GTY_CONTAINER(struct outer_struct_1);

/* Test 10: Multiple levels of indirection with various delimiters */
struct GTY(()) ultimate_test {
  /* Function returning pointer to array of function pointers */
  int (*(*(*level1)(int (*)(char[10])))[10])(float, double);
  
  /* Nested anonymous struct with complex members */
  struct GTY(()) {
    union GTY(()) {
      struct GTY(()) {
        int x[({ int y = 5; y * 2; })];
        void (*func)(void);
      } s;
      long l;
    } u;
  } anonymous;
  
  /* Multi-dimensional array with computed sizes */
  char matrix[(2+3)][(sizeof(struct outer_struct_1)*2)][10];
};

/* Test 11: Structure with GTY annotations on nested members */
struct GTY(()) outer_with_nested_annotation {
  struct GTY(()) inner_annotated {
    char* GTY((length("len"))) str;
    int len;
  } inner;
  
  struct inner_annotated* GTY((skip)) ptr_to_inner;
};

/* Test 12: Recursive structure with function pointer field */
struct GTY((chain_next)) tree_node {
  int value;
  struct tree_node *GTY((skip)) left;
  struct tree_node *GTY((skip)) right;
  int (*compare)(struct tree_node*, struct tree_node*);
  void (*traverse)(struct tree_node*, void (*)(int));
};

/* Test 13: Mixed declarations with all delimiter types */
struct GTY(()) final_mix {
  /* All three delimiters in sequence: ()[]{}([]) */
  void (*(*mixed[5])(int))[10];
  
  /* Braces containing parentheses and brackets */
  struct GTY(()) {
    int (*func)(char[]);
    double arr[5][(2+3)];
  } nested;
  
  /* Parentheses containing braces */
  union GTY(()) {
    int x;
    struct { short a; short b; } s;
  } u_array[({ int x = 3; x + 2; })];
};

/* Helper function to make the file compilable */
int main(void) {
  /* These variables are just to reference the types */
  struct outer_struct_1 os1 = {0};
  struct array_test at = {0};
  union deep_nested dn = {0};
  struct complex_member cm = {0};
  struct linked_node ln = {0};
  struct bitfield_test bt = {0};
  struct with_initializers wi = {0};
  struct container_int ci = {0};
  struct ultimate_test ut = {0};
  struct outer_with_nested_annotation owna = {0};
  struct tree_node tn = {0};
  struct final_mix fm = {0};
  
  (void)os1;
  (void)at;
  (void)dn;
  (void)cm;
  (void)ln;
  (void)bt;
  (void)wi;
  (void)ci;
  (void)ut;
  (void)owna;
  (void)tn;
  (void)fm;
  
  return 0;
}
