/* test-coverage.h - Comprehensive GTY type definitions for gengtype-state.cc coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
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
typedef struct my_struct* GTY(()) my_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct GTY(()) struct_array[5];

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
      struct lang_struct* GTY((skip)) next;
    } type2;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains pointer (TYPE_POINTER) */
  struct my_struct* GTY(()) ptr_field;
  
  /* Contains array (TYPE_ARRAY) */
  int GTY(()) array_field[8];
  
  /* Contains union (TYPE_UNION) */
  union my_union GTY(()) union_field;
  
  /* Contains callback (TYPE_CALLBACK) */
  callback_fn GTY(()) callback_field;
  
  /* Chain of structures */
  struct complex_nested* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct complex_nested* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) prev;
  
  /* Variable length array with length field */
  int count;
  struct my_struct* GTY((length("%0.count"))) var_array[1];
};

/* TYPE_STRUCT with skip option */
struct GTY(()) skip_struct {
  int important;
  void* GTY((skip)) ignored;
  struct skip_struct* GTY(()) next;
};

/* Now define the previously opaque struct (was TYPE_UNDEFINED) */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY(()) link;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct lang_struct global_lang_struct_var;
extern GTY(()) struct complex_nested* global_nested_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct opaque_struct global_opaque;

/* Structure containing pointer to itself */
struct GTY(()) self_ref {
  int value;
  struct self_ref* GTY(()) next;
};

/* Union containing pointers */
union GTY(()) pointer_union {
  struct my_struct* GTY(()) s_ptr;
  struct lang_struct* GTY(()) l_ptr;
  void* GTY(()) v_ptr;
};

/* Array of pointers */
typedef struct my_struct* GTY(()) ptr_array[4];

/* Structure with nested anonymous union */
struct GTY(()) anon_union_struct {
  int type;
  union {
    int int_val;
    double double_val;
    struct my_struct* GTY(()) struct_ptr;
  } GTY((desc("%0.type"))) data;
};

#endif /* TEST_COVERAGE_H */
