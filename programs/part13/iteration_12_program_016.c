/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with deeply nested parameter lists */
typedef int (*GTY((chain_next)) nested_callback_t)(char (*buffer)[(10 + sizeof(int))], 
                                                   struct GTY(()) inner {
                                                     int x;
                                                     void (*GTY((skip)) func)(int[][5]);
                                                   } *data);

/* Test 2: Structure with multiple nested delimiter types */
struct GTY((for_user)) complex_struct {
  /* Array with complex dimension containing parentheses */
  int arr[(sizeof(struct GTY(()) temp { int a; double b; }) + 10)];
  
  /* Function pointer array */
  void (*GTY((chain_next)) fn_array[3])(int (*)(char[][10]), 
                                        struct { 
                                          int x; 
                                          union GTY(()) { 
                                            int i; 
                                            float f; 
                                            double (*callback)(int, char); 
                                          } u;
                                        });
  
  /* Nested structure with initializer-like designators */
  struct GTY(()) nested {
    int matrix[2][(5 * sizeof(int))];
    struct GTY((desc("%1.x"))) leaf {
      int x;
      char * GTY((tag("0"))) name;
    } leaves[4];
  } GTY((skip)) inner;
};

/* Test 3: Union with complex type expressions */
union GTY((user)) complex_union {
  /* Pointer to function returning pointer to array */
  int (*(*GTY((chain_prev)) complex_fn)(void))[(10 + sizeof(double))];
  
  /* Structure containing bit-fields and arrays */
  struct GTY(()) {
    unsigned int flags : 3;
    int values[((1 << 3) - 1)];
    void (*GTY((skip)) handlers[2])(union complex_union *self);
  } parts;
  
  /* Anonymous union with nested initializer potential */
  union {
    long double big_array[1][2][3];
    struct GTY(()) { 
      short s; 
      char c[(sizeof(long) + 1)]; 
    } small;
  } GTY((desc("%0.parts.flags"))) alt;
};

/* Test 4: Typedef with multiple levels of nesting */
typedef struct GTY((user)) outer {
  /* Member with attribute containing nested parentheses */
  int GTY((special("special_type"), 
           desc("Nested %0 with array[%1.dim]"))) 
      special_value;
  
  /* Complex array of function pointers */
  void (*(*GTY((chain_next)) signal_handlers[5])(int sig)) 
        (const char *msg[(sizeof(void*) * 2)], ...);
  
  /* Union within struct with nested braces */
  union GTY(()) {
    struct GTY(()) { 
      int depth; 
      struct outer * GTY((tag("1"))) next; 
    } recursive;
    struct GTY((skip)) { 
      float matrix[3][3]; 
      void (*transform)(float[3][3]); 
    } linear;
  } variant;
  
  /* Zero-length array with computed size */
  char dynamic[0];
} outer_t;

/* Test 5: Structure with all delimiter types in single member declaration */
struct GTY((for_user)) delimiter_test {
  /* This declaration contains: (), [], and {} all nested */
  int (*(*GTY((chain_next)) extreme_member)(
        struct { 
          int count; 
          char *names[10]; 
        } *config,
        int (*compare)(const char *, const char *))
      )[]
      (void (*)(int[][10]), 
       union { 
         int i; 
         struct GTY(()) { short s; char c; } s; 
       });
};

/* Test 6: Template-like macro expansion (simulating C++ templates in C) */
#define DECLARE_VECTOR(T, N) \
  struct GTY(()) vector_##T##_##N { \
    T data[N]; \
    int (*(*allocator)(size_t))[N]; \
    struct vector_##T##_##N * GTY((tag("0"))) next; \
  }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(struct GTY(()) { int x; double y; }, 5);

/* Test 7: Structure with complex initializer (if processed by gengtype) */
static const struct GTY(()) initialized_example {
  int id;
  char * GTY((tag("1"))) name;
  struct GTY(()) {
    float values[4];
    int (*processor)(float[], int);
  } helper;
} example = {
  .id = 42,
  .name = "test",
  .helper = {
    .values = {1.0, 2.0, 3.0, 4.0},
    .processor = NULL
  }
};

/* Test 8: Multiple levels of typedef with GTY annotations */
typedef union GTY((user)) base {
  int type;
  struct GTY(()) derived {
    base * GTY((chain_next)) next;
    char * GTY((length("strlen(%h)+1"))) data;
    void (* GTY((skip)) methods[3])(struct derived *, int);
  } *derived_ptr;
} base_t;

/* Test 9: Structure containing array of structures containing arrays... */
struct GTY((for_user)) deeply_nested {
  struct GTY(()) level1 {
    struct GTY(()) level2[2];
    struct GTY(()) level2 {
      struct GTY(()) level3 {
        int data[((2 * 3) + 4)];
        void (*action)(struct level3 *, int[((2 * 3) + 4)]);
      } matrix[3][4];
      char *names[5];
    } items[10];
    int (*process)(struct level2 items[10], int index);
  } levels[5];
};

/* Test 10: Circular references with complex type expressions */
struct GTY((user)) node_a;
struct GTY((user)) node_b;

struct GTY((user)) node_a {
  int id;
  struct node_b * GTY((chain_next)) connections[5];
  void (* GTY((skip)) on_event)(struct node_a *, 
                                struct node_b *,
                                int (*callback)(char data[10]));
};

struct GTY((user)) node_b {
  char label[(sizeof(struct node_a) + 10)];
  struct node_a * GTY((chain_prev)) sources[];
  union GTY(()) {
    int (*handler)(struct node_a *, struct node_b *);
    struct GTY(()) {
      double weights[3][3];
      void (*update)(double[3][3]);
    } matrix;
  } processor;
};

/* Dummy main to make file compilable */
int main() {
  return 0;
}
