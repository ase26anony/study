/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

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

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  long field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int counter;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct * GTY(()) my_ptr;
typedef union my_union * GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct * GTY(()) ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct GTY((tag("0"))) {
      int int_val;
    } type1;
    struct GTY((tag("1"))) {
      double double_val;
      struct lang_struct * GTY((tag("0"))) next;
    } type2;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY((tag("0"))) embedded_struct;
  union my_union GTY((tag("1"))) embedded_union;
  int_array embedded_array;
  struct lang_struct * GTY((tag("2"))) lang_ptr;
  
  /* Chain pointers for linked list */
  struct complex_nested * GTY((skip)) next;
  struct complex_nested * GTY((chain_next("next"), chain_prev("prev"))) prev;
  
  /* Pointer array */
  ptr_array pointers;
  
  /* String */
  const char* GTY((length("str_len"))) dynamic_string;
  int str_len;
  
  /* Callback */
  callback_fn handler;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested *global_list_head;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct *global_lang_struct;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct * GTY(()) ptr_to_struct;
};

/* Array of unions */
typedef union my_union GTY(()) union_array[8];

/* Struct with array length determined by another field */
struct GTY(()) variable_length {
  int count;
  int GTY((length("%h.count"))) values[1];
};

#endif /* TEST_COVERAGE_H */
