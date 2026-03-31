/* complex_gty_test.c - Test file to exercise delimiter handling in gengtype-parse.cc */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter list containing arrays */
typedef void (*GTY((chain_next))
  complex_func_ptr)(int (*GTY((skip)) callback)(char[10][20]), 
                    struct GTY(()) inner { int x; double y[5]; });

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) array_test {
  int arr1[(10 + sizeof(struct GTY(()) sized { char c; int i; }))];
  long arr2[5][(2 * sizeof(void*))];
};

/* Test 3: Nested structures with multiple delimiter types */
struct GTY(()) outer_struct {
  /* Function pointer array */
  void (*GTY((tag("1"))) func_array[3])(int[][10], 
                                        struct GTY(()) param { 
                                          int a; 
                                          char b[5]; 
                                        });
  
  /* Nested union with initializer-like syntax in comments */
  union GTY((desc("%1.union_tag"))) nested_union {
    int ival;
    struct GTY(()) {
      float f;
      char str[100];
    } s;
    void (*fn)(int (*)(char[]), double);
  } u;
  
  /* Pointer to array of function pointers */
  int (*(*GTY((chain_next)) ptr_to_func_ptr_array)[5])(int, 
                                                       struct GTY(()) { 
                                                         short s; 
                                                         long l; 
                                                       });
};

/* Test 4: Multiple balanced delimiters in single declaration */
struct GTY(()) delimiter_soup {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*GTY((user)) fn_array[5])(int[][10], 
                                  union GTY((skip)) { 
                                    int x[3]; 
                                    struct { char c; } s; 
                                  });
  
  /* Complex array with nested braces */
  struct GTY(()) {
    int a;
    struct GTY(()) { 
      double d[2][2]; 
    } inner;
  } nested[3][2];
};

/* Test 5: Typedef with deeply nested parentheses */
typedef int (*GTY((chain_prev, chain_next))
  level1_func)(char (*)(int (*)(float (*)(double[5]), 
                               struct GTY(()) { int tag; }), 
                       long),
              struct GTY(()) { 
                union GTY((desc("0"))) { 
                  int i; 
                  void *GTY((skip)) p; 
                } u; 
              });

/* Test 6: Structure with bit-fields and complex array dimensions */
struct GTY(()) bitfield_test {
  unsigned int flag:1;
  unsigned int size:7;
  char data[(sizeof(struct GTY(()) header { 
                     int len; 
                     char type; 
                   }) * 10)];
  
  /* Anonymous struct with function pointer */
  struct GTY(()) {
    void (*GTY((user)) handler)(struct GTY(()) event {
                                  int type;
                                  char data[256];
                                });
    int counters[5][5];
  };
};

/* Test 7: Union containing all delimiter types */
union GTY((tag("UNION_TAG"))) all_delimiters {
  int (*GTY((skip)) func1)(int a[10], 
                          struct GTY(()) { 
                            int x; 
                            int y[5]; 
                          });
  
  struct GTY(()) struct_member {
    int (*(*complex)[3][2])(char (*)(int), 
                           void *GTY((skip)));
    union GTY((desc("1"))) {
      short s[10];
      long l;
    } u;
  } s;
  
  char multi_array[2][(sizeof(int) + 5)][10];
};

/* Test 8: Recursive structure with function pointer */
struct GTY((chain_next, chain_prev)) recursive_node {
  char *GTY((tag("0"))) name;
  struct recursive_node *GTY((skip)) next;
  struct recursive_node *GTY((skip)) prev;
  
  /* Function pointer with nested struct parameter */
  void (*GTY((user)) visit)(struct recursive_node *,
                           int (*GTY((skip)) callback)(char data[]),
                           struct GTY(()) visitor_info {
                             int id;
                             char name[50];
                           });
  
  /* Array of pointers to functions returning pointers to arrays */
  int (*(*GTY((skip)) func_table[10])(void))[5];
};

/* Test 9: Multiple GTY annotations with nested attribute lists */
struct GTY((user, chain_next("next"), chain_prev("prev"))) multi_attr {
  int value;
  struct multi_attr *GTY((skip, tag("1"))) next;
  struct multi_attr *GTY((skip, tag("2"))) prev;
  
  /* Nested with its own GTY annotation */
  struct GTY((desc("%1.nested_type"))) {
    int (*GTY((skip)) compute)(int x[3][3], 
                              struct GTY(()) matrix {
                                int rows;
                                int cols;
                                double data[10][10];
                              });
    char buffer[100];
  } nested;
};

/* Test 10: Initializer-style nested braces (in comments to show structure) */
struct GTY(()) with_init {
  int counts[3];
  struct GTY(()) coord {
    int x;
    int y;
    int z;
  } points[5];
  
  /* The actual initialization would be elsewhere, but the type
     declaration itself contains nested [] and {} */
};

/* Test 11: Template-like pattern using nested types */
struct GTY(()) container {
  /* Simulating template<typename T> */
  struct GTY(()) node {
    void *GTY((skip)) data;
    struct node *GTY((chain_next)) next;
    
    /* Function pointer with complex return type */
    struct GTY(()) result* (*GTY((user)) processor)(
      int param,
      struct GTY(()) options {
        int flags;
        char mode[20];
      });
  } *GTY((tag("0"))) head;
  
  int size;
};

/* Test 12: Mixed delimiters in typedef */
typedef union GTY((desc("%1.type"))) {
  int (*array_funcs[5])(char (*)(int), 
                       double[10],
                       struct GTY(()) { int code; });
  struct GTY(()) {
    void (*nested_func)(int (*)(char[][5]), 
                       union GTY((skip)) { 
                         short s; 
                         float f; 
                       });
  } func_struct;
} complex_union_type;

/* Main structure that references many of the above types */
struct GTY(()) master_container {
  complex_func_ptr func1 GTY((skip));
  struct array_test arrays GTY((skip));
  struct outer_struct outer GTY((skip));
  struct delimiter_soup soup GTY((skip));
  level1_func deep_func GTY((skip));
  struct bitfield_test bits GTY((skip));
  union all_delimiters delims GTY((skip));
  struct recursive_node *GTY((chain_next)) node_list;
  struct multi_attr *GTY((chain_prev)) attr_chain;
  struct with_init init_data GTY((skip));
  struct container container GTY((skip));
  complex_union_type union_var GTY((skip));
  
  /* Final complex member combining everything */
  int (*(*GTY((user)) ultimate_member)(
    struct master_container *,
    void (*)(int, char[][10], 
            struct GTY(()) { 
              int id; 
              struct { 
                double x; 
                double y; 
              } pos; 
            }),
    union GTY((desc("99"))) {
      int i;
      void *p;
    }))[10][10];
};

/* Dummy main to make file compilable */
int main() {
  return 0;
}
