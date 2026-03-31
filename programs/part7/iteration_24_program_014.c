/* Test header to cover all type kinds in gengtype-state.cc switch statement */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates an undefined type initially */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
extern GTY(()) int_array global_array;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  long field2;
  /* Nested pointer to exercise recursion */
  struct my_struct* GTY((skip)) next;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((tag("0"))) p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_LANG_STRUCT: Language-specific structure with tag-based discrimination */
enum test_node_type {
  TEST_NODE_TYPE_A,
  TEST_NODE_TYPE_B
};

struct GTY((desc("test_node_type"))) lang_struct {
  enum test_node_type code;
  /* Tagged union based on code field */
  union {
    int GTY((tag("TEST_NODE_TYPE_A"))) value_a;
    struct my_struct* GTY((tag("TEST_NODE_TYPE_B"))) value_b;
  } GTY((desc("%0.code"))) u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_UNION member */
  union my_union nested_union;
  
  /* TYPE_ARRAY member */
  int GTY(()) matrix[5][5];
  
  /* TYPE_POINTER to TYPE_USER_STRUCT */
  struct user_struct* GTY(()) user_data;
  
  /* TYPE_POINTER to TYPE_LANG_STRUCT */
  struct lang_struct* GTY(()) lang_data;
  
  /* TYPE_ARRAY of TYPE_POINTER */
  struct my_struct* GTY(()) ptr_array[8];
  
  /* TYPE_CALLBACK member */
  callback_fn handler;
  
  /* Chain pointers for chain_next/chain_prev testing */
  struct container_struct* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct container_struct* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) prev;
  
  /* String array */
  const char* GTY(()) strings[3];
};

/* Now define the previously opaque TYPE_UNDEFINED struct */
struct opaque_struct {
  int defined_now;
  struct container_struct* GTY(()) link;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container_struct global_container;
extern GTY(()) struct opaque_struct global_opaque;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct user_struct global_user_struct;

/* Variable-length array structure */
struct GTY(()) varray_struct {
  int length;
  int GTY((length("%h.length"))) data[];
};

/* Another complex type with skip option */
struct GTY(()) skip_test {
  void* GTY((skip)) skipped_ptr;
  int* GTY(()) tracked_ptr;
  struct skip_test* GTY((skip)) skip_chain;
};

#endif /* TEST_COVERAGE_H */
