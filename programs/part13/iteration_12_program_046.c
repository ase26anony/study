/* test-gty-nested-delimiters.c - Test file for gengtype parser coverage */
/* This file contains complex type definitions with nested parentheses, */
/* brackets, and braces to exercise the consume_balanced logic in */
/* gengtype-parse.cc lines 341-352 */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"

/* GTY annotations with nested attribute lists */
typedef struct GTY(()) base_struct {
  int value;
} base_struct;

/* Case 1: Function pointer with nested parameter lists - triggers '(' ')' */
typedef void (*GTY(()) complex_func_ptr)(
  int (*GTY(()) nested_callback)(char GTY((user)) buffer[10]),
  struct GTY(()) { int x; int y; } anonymous_struct
);

/* Case 2: Array with complex dimension expression - triggers '[' ']' */
struct GTY(()) array_container {
  /* Array dimension with parentheses */
  int arr1[(10 + sizeof(struct GTY(()) { int a; char b; }))];
  
  /* Multi-dimensional array */
  char arr2[5][(2 * sizeof(int))][10];
  
  /* Array of function pointers */
  void (*func_array[3])(
    int param1,
    struct GTY(()) { 
      int nested_arr[2][2]; 
    } param2
  );
};

/* Case 3: Nested structures with initializers - triggers '{' '}' */
union GTY(()) complex_union {
  struct GTY(()) {
    /* Nested structure with bit-fields */
    unsigned int flags : 4;
    unsigned int : 4;  /* unnamed bit-field */
    
    /* Array within nested struct */
    int matrix[2][2];
    
    /* Pointer to array */
    int (*ptr_to_array)[(sizeof(int) * 2)];
  } nested;
  
  /* Anonymous union within union */
  struct GTY(()) {
    /* Function pointer with complex return type */
    struct GTY(()) { 
      int results[3]; 
      char *GTY((length("strlen(%h) + 1"))) name;
    } (*processor)(
      int input,
      void (*callback)(
        char data[],
        int size
      )
    );
  } processor_wrapper;
};

/* Combined case: All delimiters in one declaration */
struct GTY(()) all_delimiters_example {
  /* Contains: ( ) [ ] { } */
  void (*(*complex_member)[5])(
    int param1[][10],
    struct GTY(()) {
      int x;
      union GTY(()) {
        char c;
        int i;
      } value;
    } param2
  );
  
  /* Initializer-like syntax in array dimension (GCC extension) */
  int sized_array[sizeof(struct { int a; double b; })];
};

/* Chain of GTY annotations with nested attributes */
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
  int data;
  struct linked_node *GTY((skip)) next;
  struct linked_node *prev;
  
  /* Nested structure with its own GTY annotation */
  struct GTY(()) metadata {
    char *GTY((length("strlen(%h) + 1"))) description;
    int tags[3];
  } meta;
};

/* Template-like pattern using nested parentheses */
typedef struct GTY(()) template_like {
  /* Simulating template parameters with function pointers */
  void (*compare)(
    const void *a,
    const void *b,
    int (*validator)(const char criteria[])
  );
  
  /* Array with computed size */
  unsigned char buffer[sizeof(void (*)()) * 2];
} template_like_t;

/* Recursive structure with complex nested types */
struct GTY(()) tree_node {
  int value;
  
  /* Array of pointers to child nodes */
  struct tree_node *GTY((length("%h.child_count"))) children[10];
  int child_count;
  
  /* Callback function pointer */
  void (*GTY((callback))) visit_handler(
    struct tree_node *node,
    void (*pre_action)(int depth),
    void (*post_action)(int results[], int count)
  );
  
  /* Nested union with anonymous struct */
  union GTY(()) {
    struct GTY(()) {
      int x_coord;
      int y_coord;
      int z_coord;
    } position;
    
    struct GTY(()) {
      char *name;
      int id;
    } info;
  } data;
};

/* Multiple levels of nesting */
struct GTY(()) level1 {
  struct GTY(()) level2 {
    struct GTY(()) level3 {
      int arr[(
        sizeof(struct { 
          char a; 
          short b; 
        }) + 1
      )];
      
      void (*func)(
        int a[(
          sizeof(int[2]) * 3
        )],
        struct { 
          int x; 
        } param
      );
    } deepest;
    
    union GTY(()) {
      int i;
      struct { 
        char c[4]; 
      } chars;
    } value;
  } middle;
  
  /* Pointer to function returning pointer to array */
  int (*(*get_matrix)(void))[2][2];
};

/* Test case with all delimiter types in sequence */
struct GTY(()) delimiter_sequence {
  /* Pattern: ( [ { } ] ) [ ( ) ] { ( [ ] ) } */
  void (*start)(int first[3]);  /* ( [ ] ) */
  
  struct GTY(()) {
    int middle;
  } block;  /* { } */
  
  char end[(sizeof(int) + 1)];  /* [ ] */
};

/* Additional stress test: deeply nested parentheses */
typedef void (*(*(*deeply_nested_func)(void))(
  int,
  char (*names[])[20]
))(
  struct GTY(()) config {
    int values[10];
  }
);

/* Main structure collecting all test cases */
struct GTY(()) gty_test_suite {
  complex_func_ptr func_test;
  struct array_container array_test;
  union complex_union union_test;
  struct all_delimiters_example combined_test;
  struct linked_node *list_test;
  template_like_t template_test;
  struct tree_node *tree_test;
  struct level1 nesting_test;
  struct delimiter_sequence sequence_test;
  deeply_nested_func deep_test;
};

/* Global instance for gengtype to process */
static GTY(()) struct gty_test_suite global_test_suite;

/* Function to ensure the types are referenced (avoid compiler warnings) */
void use_types(void) {
  /* Reference all types to ensure they're not optimized away */
  volatile int dummy = sizeof(global_test_suite);
  dummy += sizeof(base_struct);
  (void)dummy;
}
