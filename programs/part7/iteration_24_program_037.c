/* test-gtype-coverage.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration to create undefined type reference */
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
  double field2;
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int tag;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  double d;
  void* GTY((skip)) p;
  struct my_struct* GTY((skip)) s;
};

/* TYPE_POINTER: Pointer type definition */
typedef struct my_struct* GTY(()) my_struct_ptr;

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
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
    char* GTY((tag("2"))) name;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* TYPE_ARRAY within struct */
  int GTY(()) matrix[3][3];
  
  /* TYPE_POINTER within struct */
  struct complex_nested* GTY((skip)) next;
  
  /* TYPE_UNION within struct */
  union my_union data;
  
  /* TYPE_STRUCT within struct */
  struct my_struct embedded;
  
  /* TYPE_ARRAY of pointers */
  struct lang_struct* GTY((skip)) lang_nodes[4];
  
  /* TYPE_CALLBACK within struct */
  callback_fn handler;
};

/* Chain structures for chain_next/chain_prev testing */
struct GTY((chain_next("%0.next"), chain_prev("%0.prev"))) chain_node {
  int id;
  struct chain_node* GTY((skip)) next;
  struct chain_node* GTY((skip)) prev;
  void* GTY((skip)) data;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested* global_nested_ptr;
extern GTY(()) struct chain_node* global_chain_head;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct* global_lang_struct;

/* Now define the previously opaque struct to resolve TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int resolved;
  struct my_struct* GTY((skip)) link;
};

/* Array of undefined->defined type transition */
extern GTY(()) struct opaque_struct* opaque_array[8];

/* Parameterized structure with length option */
struct GTY(()) variable_len_struct {
  int count;
  int GTY((length("%0.count"))) data[];
};

#endif /* TEST_GTYPE_COVERAGE_H */
