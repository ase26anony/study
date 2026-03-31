/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.c"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((chain_next, chain_prev))
  complex_func_ptr)(int (*GTY(()) nested_callback)(char[10]), 
                    struct GTY(()) inner_struct { int x; });

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) outer_struct {
  int arr[(10 + sizeof(struct GTY(()) size_calc { char data[20]; }))];
  void (*func_array[5])(int[][10]);
};

/* Test 3: Deeply nested structure with multiple delimiter types */
union GTY(()) deep_nested {
  struct GTY(()) level1 {
    int (*level1_func)(struct GTY(()) level2 {
      char *level2_array[3][(sizeof(int) * 2)];
      union GTY(()) level3 {
        void (*level3_callback[2])(
          int param1,
          struct GTY(()) param_struct {
            int field1;
            char field2[((5 * 2) + 1)];
          } param2
        );
        struct GTY(()) alt_level3 {
          int alt_field;
        } alt;
      } u;
    } *l2_ptr);
  } s;
  
  /* Test 4: Complex initializer with nested braces */
  struct GTY(()) with_init {
    int matrix[2][3];
    struct GTY(()) point {
      int x;
      int y;
    } points[2];
  } init_example GTY((user)) = {
    .matrix = {{1, 2, 3}, {4, 5, 6}},
    .points = {{.x = 10, .y = 20}, {.x = 30, .y = 40}}
  };
};

/* Test 5: Multiple balanced delimiters in single declaration */
struct GTY(()) all_delimiters {
  /* Contains: (*, ), [5], (, [][10], ) */
  void (*fn_array[5])(int[][10]);
  
  /* Contains: {, {, }, }, [, ], =, {, {, }, } */
  struct GTY(()) nested_braces {
    struct GTY(()) inner { int a; } in;
    int arr[3];
  } nb GTY((skip)) = { .in = { .a = 42 }, .arr = {1, 2, 3} };
  
  /* Complex function pointer type */
  int (*(*complex_fp)(void (*)(int)))[10];
};

/* Test 6: Template-like pattern using nested parentheses */
typedef struct GTY(()) template_like {
  void *GTY((length("((struct template_like *)x)->count"))) data;
  int count;
  
  /* Function with attributes in parameter */
  void (*processor)(
    __attribute__((aligned(16))) char *buffer,
    int sizes[((sizeof(void*) * 8) / 2)]
  );
} template_like_t;

/* Test 7: Union with anonymous struct containing bitfields and arrays */
union GTY(()) bitfield_test {
  struct GTY(()) {
    unsigned int flag : 1;
    unsigned int value : 15;
    unsigned char bytes[((16 - 1 - 15) / 8)];
  } bits;
  
  struct GTY(()) alt_view {
    int (*comparator)(const void *, const void *);
    struct GTY(()) key {
      char *name;
      int id;
    } keys[2];
  } view;
};

/* Test 8: Recursive structure with function pointer */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((tag("0"))) left;
  struct tree_node *GTY((tag("1"))) right;
  
  /* Visitor function pointer with complex signature */
  void (*visitor)(
    struct tree_node *node,
    void (*callback)(int result, char output[256]),
    int depth
  );
};

/* Test 9: Array of function pointers returning pointers to arrays */
typedef int (*(*array_of_funcs[3])(void))[5];

struct GTY(()) container {
  array_of_funcs funcs;
  
  /* Nested initializer with designators */
  struct GTY(()) config {
    int modes[4];
    struct GTY(()) setting {
      char name[32];
      int value;
    } settings[2];
  } cfg GTY((user)) = {
    .modes = {[0] = 1, [2] = 3, [3] = 4},
    .settings = {
      [0] = {.name = "mode1", .value = 100},
      [1] = {.name = "mode2", .value = 200}
    }
  };
};

/* Test 10: Multiple levels of GTY annotations */
struct GTY(()) level0 {
  struct GTY((chain_next("next"), chain_prev("prev"))) level1 {
    struct level1 *next;
    struct level1 *prev;
    
    union GTY((desc("2"))) level2 {
      int as_int;
      struct GTY((tag("0"))) {
        float x;
        float y;
      } as_point;
      
      struct GTY((tag("1"))) {
        char *GTY((length("len"))) str;
        int len;
      } as_string;
    } data;
  } *list;
};

/* Main function to make the file compilable */
int main(void) {
  return 0;
}
