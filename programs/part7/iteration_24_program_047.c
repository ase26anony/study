/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration for opaque type */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  double field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  double d;
  void* GTY((tag("0"))) p;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct GTY(()) struct_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct GTY((tag("0"))) {
      int int_val;
    } t1;
    struct GTY((tag("1"))) {
      double double_val;
    } t2;
  } u;
};

/* TYPE_SCALAR: Fundamental scalar types as GC roots */
extern GTY(()) int global_scalar;
extern GTY(()) long global_long;
extern GTY(()) double global_double;

/* TYPE_STRING: String types */
extern GTY(()) const char* global_string;
extern GTY(()) char* mutable_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
typedef int (* GTY(()) int_callback)(int, double);

/* Complex nested structure to exercise recursive traversal */
struct GTY(()) complex_nested {
  /* Contains a pointer */
  struct my_struct* GTY(()) ptr_field;
  
  /* Contains an array */
  int GTY(()) array_field[8];
  
  /* Contains a union */
  union my_union GTY(()) union_field;
  
  /* Chain pointer for linked list */
  struct complex_nested* GTY((chain_next("%h.next"))) next;
  
  /* Callback field */
  callback_fn GTY(()) handler;
  
  /* String field */
  const char* GTY(()) name;
};

/* Another structure with skip/length options */
struct GTY(()) variable_length {
  int GTY(()) length;
  int GTY((length("%h.length * 2"))) data[1];
};

/* Structure with conditional pointers */
struct GTY(()) conditional_struct {
  int type;
  union GTY((desc("%0.type"))) {
    struct GTY((tag("0"))) {
      int* GTY(()) int_ptr;
    } case_int;
    struct GTY((tag("1"))) {
      double* GTY(()) double_ptr;
    } case_double;
    struct GTY((tag("2"))) {
      struct my_struct* GTY(()) struct_ptr;
    } case_struct;
  } u;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested* global_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) callback_fn global_callback;

/* Now define the previously opaque type */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY(()) link;
};

#endif /* TEST_COVERAGE_H */
