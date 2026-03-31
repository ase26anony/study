/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef int (*GTY(()) nested_func_ptr)(void (*)(char[10]), struct { int x; });

struct GTY(()) outer_struct {
  /* Case '(': Function pointer with complex signature */
  void (*GTY((skip)) complex_fn)(
    int (*callback)(int[5][10]), 
    struct inner { int y; } GTY(()) *ptr
  );
  
  /* Case '[': Multi-dimensional array with computed size */
  char buffer[(sizeof(struct outer_struct) + 10) * 2];
  
  /* Case '{': Nested anonymous struct */
  struct {
    int GTY((tag("0"))) depth;
    union {
      long GTY((default)) l;
      double GTY((skip)) d;
    } GTY(()) data;
  } GTY(()) inner;
};

/* Test 2: Template-like pattern using macros */
#define DECLARE_VECTOR(TYPE, SIZE) \
  struct GTY(()) vector_##TYPE { \
    TYPE GTY((length(SIZE))) items[SIZE]; \
    struct { \
      int (*GTY((skip)) allocator)(TYPE (*)[SIZE]); \
    } GTY(()) meta; \
  }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(struct outer_struct*, 5);

/* Test 3: Chain of structures with nested arrays */
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
  struct linked_node *GTY((skip)) next;
  struct linked_node *GTY((skip)) prev;
  
  /* Complex array of function pointers */
  void (*GTY((skip)) handlers[5])(
    struct linked_node *GTY((skip)),
    int matrix[][10]
  );
  
  /* Nested initializer-like structure */
  struct config {
    int GTY((default)) flags;
    char GTY((length("len"))) *name;
    int len;
  } GTY(()) settings;
};

/* Test 4: Union with all delimiter types */
union GTY(()) all_delimiters {
  /* Parentheses in function pointer */
  int (*GTY((skip)) func)(union all_delimiters *GTY((skip)) self);
  
  /* Brackets in array */
  struct outer_struct GTY((skip)) items[3];
  
  /* Braces in anonymous struct */
  struct {
    int GTY((default)) a;
    int GTY((default)) b;
    int GTY((default)) c;
  } GTY(()) triple;
  
  /* Combined: array of function pointers returning structs */
  struct { int x; } GTY(()) (*array_of_funcs[2])(
    int param1,
    char param2[]
  );
};

/* Test 5: Deeply nested example */
typedef struct GTY(()) deeply_nested {
  /* Function returning pointer to array of function pointers */
  int (*(*GTY((skip)) complex_member)(
    void (*)(char, short, int, long)
  ))[10];
  
  /* Struct containing union containing struct... */
  struct level1 {
    union level2 {
      struct level3 {
        int (*GTY((skip)) level4[3])(
          struct level3 *GTY((skip)),
          int arr[sizeof(struct level1)]
        );
      } GTY(()) l3;
    } GTY(()) l2;
  } GTY(()) l1;
  
  /* Initializer with designators */
  struct init_example {
    int GTY((default)) values[5];
    struct {
      char GTY((length("len"))) *str;
      int len;
    } GTY(()) str_info;
  } GTY(()) init;
} deeply_nested_t;

/* Test 6: Multiple balanced delimiters in sequence */
struct GTY(()) sequence_test {
  /* All three delimiters in one declaration */
  void (*(*GTY((skip)) fn_matrix[3][2])(
    int (*)(char[10]),
    struct { int tag; } GTY(())
  ))[5];
  
  /* Nested attribute lists */
  struct GTY((chain_next("n"), chain_prev("p"))) chain_item {
    struct chain_item *GTY((skip)) n;
    struct chain_item *GTY((skip)) p;
    int GTY((default)) value;
  } GTY(()) *GTY((skip)) chain_head;
};

/* Test 7: Recursive structures with arrays of pointers */
struct GTY(()) tree_node {
  int GTY((default)) type;
  
  /* Array of pointers to same type */
  struct tree_node *GTY((length("child_count"), skip)) children[
    (sizeof(struct tree_node) > 32) ? 4 : 8
  ];
  int child_count;
  
  /* Function pointer with complex return type */
  struct {
    int start;
    int end;
  } GTY(()) (*GTY((skip)) get_range)(struct tree_node *GTY((skip)));
};

/* Test 8: Mixed declarations with GTY annotations at different levels */
struct GTY(()) container {
  /* GTY on typedef inside struct */
  typedef struct GTY(()) inner_type {
    int (*GTY((skip)) compare)(
      const struct inner_type *GTY((skip)),
      const struct inner_type *GTY((skip))
    );
  } inner_t;
  
  inner_t GTY((skip)) items[10];
  
  /* Anonymous union with GTY on members */
  union {
    int GTY((default)) as_int;
    double GTY((skip)) as_double;
    struct GTY(()) {
      char GTY((length("len"))) *data;
      int len;
    } GTY(()) as_struct;
  } GTY(()) variant;
};

/* Test 9: Bitfields with complex expressions (still uses braces) */
struct GTY(()) bitfield_test {
  unsigned int GTY((bitfield("3"))) flags:3;
  unsigned int GTY((default)) :5;  /* unnamed bitfield */
  unsigned int GTY((bitfield("10"))) size:10;
  
  /* Following bitfield with array */
  char GTY((length("size"))) name[];
};

/* Test 10: Forward declarations and opaque pointers */
struct GTY(()) forward_decl_test {
  struct opaque *GTY((skip)) opaque_ptr;
  void (*GTY((skip)) opaque_callback)(struct opaque *GTY((skip)));
};

struct opaque {
  int GTY((default)) data;
  struct forward_decl_test *GTY((skip)) back_ref;
};

/* Main function to make file compilable */
int main() {
  return 0;
}
