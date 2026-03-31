/* complex_gty_test.c - Test file for gengtype delimiter parsing */

/* This file contains complex type definitions with nested parentheses,
   brackets, and braces to exercise the consume_balanced logic in
   gengtype-parse.cc lines 341-352. */

#include "gtype-desc.c"

/* GTY annotation with nested attribute lists */
typedef struct GTY((chain_next, chain_prev)) linked_node {
  struct linked_node *GTY((skip)) next;
  struct linked_node *prev;
  int data;
} linked_node_t;

/* Complex function pointer type with nested parentheses */
typedef void (*GTY(()) complex_callback)(
    int (*GTY((callback)) nested_callback)(char GTY((length("10"))) buffer[10]),
    struct GTY(()) { int x; double y; } param
);

/* Structure with array containing complex size expression */
struct GTY(()) array_container {
  /* Array with size containing parentheses expression */
  int arr1[(10 + sizeof(struct linked_node))];
  
  /* Multi-dimensional array */
  double matrix[5][10];
  
  /* Array of function pointers */
  complex_callback callbacks[3];
};

/* Union with nested structures and braces */
union GTY(()) complex_union {
  struct GTY(()) {
    int type;
    union GTY(()) {
      int int_val;
      double double_val;
      char *GTY((tag("0"))) string_val;
    } data;
  } variant;
  
  struct GTY(()) {
    long long big_array[100];
    struct GTY(()) {
      short small;
      char tiny;
    } nested;
  } bulk_data;
};

/* Template-like pattern using nested types */
struct GTY(()) template_wrapper {
  /* Pointer to array of pointers to functions returning pointers */
  int *(*(*GTY((skip)) complex_array[5])(int))[10];
  
  /* Nested structure with bit-fields (uses braces) */
  struct GTY(()) {
    unsigned int flag:1;
    unsigned int count:7;
    unsigned int:0;  /* Force alignment */
    unsigned long value:24;
  } flags;
};

/* Even more complex: function pointer returning pointer to array */
typedef struct GTY(()) *(*(*ultra_complex_fn)(
    int arg1,
    struct GTY(()) { 
      int x; 
      struct GTY(()) { 
        char c; 
        double d; 
      } inner;
    } arg2
))[10];

/* Structure with initializer-like nested braces */
struct GTY(()) with_initializer {
  int values[5];
  struct GTY(()) {
    char name[50];
    int id;
  } metadata;
  
  /* Simulate a complex initializer pattern */
  struct GTY(()) {
    union GTY(()) {
      int as_int;
      float as_float;
    } value;
    int type;
  } variant_data[3];
};

/* Chain of nested structures with all delimiter types */
struct GTY(()) level1 {
  int (*level1_func)(char level1_array[((sizeof(int) * 2) + 1)]);
  struct GTY(()) level2 {
    void (*level2_func[2])(
        struct GTY(()) { 
          int a; 
          int b[5]; 
        } param
    );
    struct GTY(()) level3 {
      struct GTY(()) {
        union GTY(()) {
          int x;
          long y;
        } u;
        int count;
      } data;
      int (*final_func)(
          int,
          char *[],
          struct GTY(()) { 
            double matrix[3][3]; 
          }
      );
    } l3;
  } l2;
};

/* Test case with deeply nested parentheses */
typedef int (*(*(*deeply_nested_func_ptr)(
    void (*(*arg1)(int (*)(char)))[5],
    struct GTY(()) { 
      int (*(*member)[10])(float); 
    }
))(double))(char);

/* Structure containing all delimiter types in one member */
struct GTY(()) all_delimiters {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*fn_array[5])(int[][10]);
  
  /* Contains: {, {, }, } */
  struct GTY(()) { 
    struct GTY(()) { 
      int x; 
    } inner; 
  } nested_struct;
  
  /* Contains: (, (, ), ) */
  int (*(*double_ptr))(void);
  
  /* Contains: [, (, ), ] */
  int (*array_of_funcs[3])(void);
  
  /* Contains: {, [, ], } */
  struct GTY(()) { 
    int values[5]; 
  } with_array;
};

/* Additional stress test: multiple levels of nesting */
struct GTY(()) stress_test {
  /* Function returning pointer to array of pointers to functions */
  int (*(*(*stress_func1)(
      int (*(*stress_arg1)[5])(
          struct GTY(()) { 
            char data[100]; 
          }
      )
  ))[10])(float);
  
  /* Nested anonymous structures and unions */
  union GTY(()) {
    struct GTY(()) {
      int (*(*func_ptr))(int[((2*3)+4)]);
      struct GTY(()) {
        union GTY(()) {
          short s;
          char c[2];
        } u;
      } inner;
    } s;
    long long big_value;
  } u;
};

/* Global variable with complex type */
struct GTY(()) all_delimiters global_complex_var;

/* Main function to make the file compilable */
int main() {
  /* The GTY annotations are for gengtype only; 
     they expand to nothing in normal compilation */
  return 0;
}
