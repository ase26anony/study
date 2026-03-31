/* test-coverage.h - Comprehensive GTY annotations for gengtype coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type as GC root */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  long field2;
  struct my_struct* GTY((skip)) next;  /* Using skip option */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* GTY((tag("0"))) p;  /* Using tag option */
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_code { TEST_NODE_A, TEST_NODE_B, TEST_NODE_C };

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_code code;
  union GTY((desc("%1.code"))) lang_union {
    int GTY((tag("0"))) ival;
    float GTY((tag("1"))) fval;
    struct lang_struct* GTY((tag("2"))) child;
  } u;
  struct lang_struct* GTY((chain_next("%0.u.child"))) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY(()) embedded_struct;
  union my_union GTY(()) embedded_union;
  int_array embedded_array;
  struct lang_struct* GTY(()) lang_ptr;
  callback_fn callback_field;
  
  /* Chain of structures */
  struct complex_nested* GTY((chain_next("%h.next"))) next;
  struct complex_nested* GTY((chain_prev("%h.prev"))) prev;
  
  /* Variable length array using length option */
  int count;
  struct my_struct* GTY((length("%h.count"))) variable_array[1];
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) struct complex_nested* global_complex;
extern GTY(()) int_array global_int_array;

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
  int defined_field;
  struct opaque_struct* GTY(()) next;
};

/* Array of pointers with skip option */
struct GTY(()) pointer_container {
  void* GTY((skip("skip_func"))) skipped_pointer;
  struct my_struct* GTY(()) regular_pointer;
};

/* Union with desc and tag for language type testing */
union GTY((desc("TREE_CODE"))) tree_like {
  int GTY((tag("0"))) intval;
  double GTY((tag("1"))) doubleval;
  struct lang_struct* GTY((tag("2"))) node;
};

#endif /* TEST_COVERAGE_H */
