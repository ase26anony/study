/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) callback)(char[10]), 
                    struct GTY(()) { int x; } param);

struct GTY(()) test_struct_1 {
  /* Function pointer array with complex signature */
  complex_func_ptr GTY((skip)) func_array[5];
  
  /* Nested function pointer declaration */
  int (*GTY((tag("1"))) nested_fp)(char (*)[(10 + sizeof(struct test_struct_1))]);
};

/* Test 2: Deeply nested parentheses in array dimensions */
struct GTY(()) test_struct_2 {
  /* Array with expression containing parentheses */
  int arr1[(10 * (sizeof(int) + (5 * 2)))];
  
  /* Multi-dimensional array with nested calculations */
  char arr2[5][(3 + (2 * (1 + 1)))];
  
  /* Pointer to array with parenthesized size */
  int (*GTY((length("len"))) ptr_to_arr)[(sizeof(struct test_struct_2) + 8)];
};

/* Test 3: Nested structures with various delimiters */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct_1 {
    int x;
    /* Array in nested struct */
    double data[10];
    
    /* Function pointer member */
    void (*GTY(()) handler)(int, char);
  } inner1;
  
  union GTY(()) inner_union {
    int i;
    /* Array in union */
    char str[20];
    
    /* Pointer with attributes */
    struct outer_struct* GTY((reorder, skip)) next;
  } inner2;
  
  /* Anonymous struct with bitfield */
  struct GTY(()) {
    unsigned int flag:1;
    unsigned int value:(8 * sizeof(int) - 1);
  } bits;
};

/* Test 4: Complex initializer with nested braces */
static struct GTY(()) initialized_struct {
  int id;
  char name[50];
  struct GTY(()) {
    float x;
    float y;
  } point;
  
  /* Array with designated initializers */
  int matrix[2][3];
} GTY(()) global_var = {
  .id = 100,
  .name = "test",
  .point = { .x = 1.0, .y = 2.0 },
  .matrix = { 
    {1, 2, 3}, 
    {4, 5, 6} 
  }
};

/* Test 5: Template-like macro with nested delimiters */
#define DECLARE_VECTOR_TYPE(TYPE, SIZE) \
  struct GTY(()) vector_##TYPE { \
    TYPE GTY((length(#SIZE))) data[(SIZE)]; \
    int (*GTY(()) compare)(TYPE*, TYPE*); \
  }

/* Instantiate with nested expressions */
DECLARE_VECTOR_TYPE(int, (10 + 5));
DECLARE_VECTOR_TYPE(char*, (sizeof(struct outer_struct) / 8));

/* Test 6: Multiple delimiter types in single declaration */
struct GTY(()) ultimate_test {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*GTY((user)) fn_array[5])(int[][10]);
  
  /* Mixed delimiters with attributes */
  struct GTY((for_user)) {
    int (*GTY(()) callback)(void);
    char buffer[256];
  } GTY(()) *GTY((chain_next)) container;
  
  /* Complex array of function pointers */
  int (*(*GTY(()) signal_handlers[3])(int, ...))[5];
};

/* Test 7: Recursive structure with nested delimiters */
struct GTY(()) tree_node {
  char* GTY((length("len"))) data;
  
  /* Array of child pointers */
  struct tree_node* GTY((length("child_count"))) children[10];
  
  /* Function pointer for traversal */
  void (*GTY(()) visit)(struct tree_node*, 
                        void (*)(char*, int));
  
  /* Nested union with array */
  union GTY(()) {
    int int_val;
    double dbl_val;
    char str_val[100];
  } value;
};

/* Test 8: Attribute lists with nested parentheses */
typedef struct GTY((chain_next("next"), 
                    chain_prev("prev"),
                    user("my_type"))) linked_node {
  int data;
  struct linked_node* GTY((skip)) next;
  struct linked_node* GTY((skip)) prev;
  
  /* Callback with attributes */
  void (*GTY((user("callback"))) notify)(struct linked_node*, 
                                          int (*)(const char*));
} linked_node_t;

/* Test 9: Multiple levels of nesting */
struct GTY(()) level1 {
  struct GTY(()) level2 {
    struct GTY(()) level3 {
      int (*GTY(()) deep_func[2][2])(
        struct { 
          int a; 
          int b[5]; 
        }, 
        char (*)[((10) + (20))]
      );
    } l3;
    
    union GTY(()) {
      int x;
      struct level1* GTY((reorder)) parent;
    } link;
  } l2;
  
  /* Array of anonymous structs */
  struct GTY(()) {
    int id;
    char name[(50 + 10)];
  } items[5];
};

/* Test 10: Initializer with all delimiter types */
static struct GTY(()) mixed_init {
  /* Function pointer initialized to NULL */
  int (*GTY(()) func_ptr)(int, char) = NULL;
  
  /* Array with nested initializer */
  struct GTY(()) {
    int codes[3];
    char* desc;
  } entries[2] = {
    [0] = { .codes = {1, 2, 3}, .desc = "first" },
    [1] = { .codes = {4, 5, 6}, .desc = "second" }
  };
  
  /* Multi-dimensional array initializer */
  int matrix[2][3] = {
    {10 * (2 + 1), 20, 30},
    {40, 50, 60}
  };
} GTY(()) mixed_var;

/* Main function to make file compilable */
int main(void) {
  /* Reference variables to avoid unused warnings */
  (void)global_var.id;
  (void)mixed_var.matrix[0][0];
  
  return 0;
}
