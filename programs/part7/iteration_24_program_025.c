/* test-coverage.h - Comprehensive GTY type definitions for gengtype-state.cc coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
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
  const char* GTY(()) field2;
  struct opaque_struct* GTY(()) next;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  /* User will provide marking routine */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY(()) p;
  struct my_struct* GTY(()) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
enum test_node_type { TEST_NODE_A, TEST_NODE_B, TEST_NODE_C };

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_type code;
  union GTY((tag("0"))) {
    int GTY((tag("TEST_NODE_A"))) value;
    struct my_struct* GTY((tag("TEST_NODE_B"))) child;
    const char* GTY((tag("TEST_NODE_C"))) name;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Chain of structures */
  struct complex_nested* GTY((skip)) next;
  struct complex_nested* GTY((skip)) prev;
  
  /* Array of pointers */
  struct my_struct* GTY(()) ptr_array[5];
  
  /* Union containing different types */
  union my_union data;
  
  /* Language structure */
  struct lang_struct lang_data;
  
  /* Callback function */
  callback_fn handler;
  
  /* Variable length array (uses length option) */
  int GTY((length("%0.dynamic_length"))) *dynamic_array;
  int dynamic_length;
};

/* Now define the previously undefined type */
struct GTY(()) opaque_struct {
  int id;
  struct complex_nested* GTY(()) complex;
  my_ptr alias_ptr;
};

/* Global variables using all types */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct complex_nested global_complex;
extern GTY(()) struct opaque_struct global_opaque;

/* Struct with chain_next/chain_prev options */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_struct {
  int value;
  struct chain_struct* GTY((skip)) next;
  struct chain_struct* GTY((skip)) prev;
};

/* Array of unions */
typedef union my_union GTY(()) union_array[3];

/* Pointer to array */
typedef int_array* GTY(()) array_ptr;

#endif /* TEST_COVERAGE_H */
