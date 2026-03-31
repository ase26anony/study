/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
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
  struct my_struct* GTY((skip)) next;  /* Using skip option */
};

/* TYPE_USER_STRUCT: User-defined marking routine */
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

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("TEST_NODE_TYPE1"))) child;
    int GTY((tag("TEST_NODE_TYPE2"))) value;
  } u;
  struct lang_struct* GTY((chain_next)) chain;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY((skip)) base_struct;      /* TYPE_STRUCT */
  union my_union GTY((skip)) data_union;         /* TYPE_UNION */
  struct lang_struct* GTY((skip)) lang_ptr;      /* TYPE_POINTER to TYPE_LANG_STRUCT */
  int_array numbers;                             /* TYPE_ARRAY */
  callback_fn handler;                           /* TYPE_CALLBACK */
  const char* GTY((skip)) name;                  /* TYPE_STRING */
  
  /* Chain for linked list */
  struct complex_nested* GTY((chain_next)) next;
  struct complex_nested* GTY((chain_prev)) prev;
};

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY((skip)) link;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct lang_struct* global_lang_struct_ptr;
extern GTY(()) struct complex_nested* global_complex_list;
extern GTY(()) struct opaque_struct global_opaque;
extern GTY(()) int_array global_int_array;

/* Struct with length option for arrays */
struct GTY(()) variable_length_struct {
  int count;
  struct my_struct* GTY((length("%h.count"))) items;
};

/* Union with nested structures */
union GTY(()) nested_union {
  struct {
    int type;
    void* GTY((skip)) data;
  } GTY((skip)) s;
  long long int big_value;
};

#endif /* TEST_COVERAGE_H */
