/* test-coverage.h - Comprehensive GTY type definitions for gengtype-state.cc coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates an undefined type initially */
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
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tagging */
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
      struct lang_struct* GTY((tag("0"))) child;
    } type2;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* TYPE_ARRAY nested */
  int GTY(()) matrix[3][3];
  
  /* TYPE_POINTER nested */
  struct my_struct* GTY(()) data_ptr;
  
  /* TYPE_UNION nested */
  union my_union value;
  
  /* TYPE_STRUCT nested */
  struct {
    int GTY(()) nested_field;
    char* GTY(()) nested_string;
  } inner;
  
  /* Chain pointers for chain_next/chain_prev testing */
  struct complex_nested* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct complex_nested* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) prev;
  
  /* Array of pointers */
  struct lang_struct* GTY(()) lang_ptrs[4];
  
  /* Callback field */
  callback_fn handler;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested* global_complex_ptr;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct global_lang_struct;

/* TYPE_UNDEFINED: Now define the previously forward-declared struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY(()) ptr;
};

/* Struct with length option for variable-sized array */
struct GTY(()) var_size_struct {
  int count;
  int GTY((length("%0.count"))) data[];
};

/* Union with pointer to itself for recursion */
union GTY(()) recursive_union {
  int value;
  union recursive_union* GTY((skip)) next;
};

#endif /* TEST_COVERAGE_H */
