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
  void* GTY((skip)) field2;  /* Use skip option */
  struct opaque_struct* GTY((tag("0"))) opaque_ptr;  /* Forward reference */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  /* User will provide marking routine */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_LANG_STRUCT: Language-specific structure with descriptor */
enum test_node_codes {
  TEST_NODE_ARRAY,
  TEST_NODE_STRUCT,
  TEST_NODE_UNION
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) lang_union {
    struct my_struct* GTY((tag("TEST_NODE_ARRAY"))) as_array;
    struct lang_struct* GTY((tag("TEST_NODE_STRUCT"))) as_struct;
    union my_union* GTY((tag("TEST_NODE_UNION"))) as_union;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* Chain of structures */
  struct container_struct* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct container_struct* GTY((skip)) prev;
  
  /* Array of pointers */
  struct my_struct* GTY(()) ptr_array[5];
  
  /* Union field */
  union my_union data;
  
  /* Language structure */
  struct lang_struct lang_data;
  
  /* Variable length array (uses length option) */
  int count;
  int GTY((length("%0.count"))) var_array[1];
};

/* Now define the previously opaque structure */
struct GTY(()) opaque_struct {
  int defined_now;
  struct container_struct* GTY(()) container;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct container_struct* global_container;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct opaque_struct global_opaque;

/* Array of different types */
typedef struct GTY(()) type_collection {
  struct my_struct* GTY(()) s;
  union my_union* GTY(()) u;
  struct lang_struct* GTY(()) l;
  callback_fn cb;
} type_collection;

extern GTY(()) type_collection type_array[3];

/* Structure with callback field */
struct GTY(()) has_callback {
  const char* GTY(()) name;
  callback_fn GTY(()) handler;
  void* GTY(()) user_data;
};

#endif /* TEST_COVERAGE_H */
