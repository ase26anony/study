/* Test header to cover all gengtype-state.cc switch cases */

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

/* TYPE_USER_STRUCT: User-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((tag("0"))) p;  /* Using tag option */
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_type {
  TEST_NODE_TYPE_A,
  TEST_NODE_TYPE_B
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_type code;
  union GTY((desc("%1.code"))) {
    struct GTY((tag("TEST_NODE_TYPE_A"))) {
      int value_a;
      char* GTY((length("strlen(%h.data) + 1"))) data;  /* Using length option */
    } type_a;
    struct GTY((tag("TEST_NODE_TYPE_B"))) {
      double value_b;
      struct lang_struct* GTY((chain_next("%h.next"))) next;  /* Using chain_next */
    } type_b;
  } GTY((desc("1"))) u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains multiple type kinds */
  struct my_struct GTY(()) embedded_struct;      /* TYPE_STRUCT */
  union my_union GTY(()) embedded_union;         /* TYPE_UNION */
  my_ptr GTY(()) pointer_field;                  /* TYPE_POINTER */
  int_array GTY(()) array_field;                 /* TYPE_ARRAY */
  struct lang_struct* GTY(()) lang_ptr;          /* TYPE_LANG_STRUCT pointer */
  callback_fn GTY(()) callback_field;            /* TYPE_CALLBACK */
  const char* GTY(()) string_field;              /* TYPE_STRING */
};

/* Chain structure for chain_next/chain_prev testing */
struct GTY(()) chain_node {
  int value;
  struct chain_node* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct chain_node* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) prev;
};

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
  int resolved_field;
  struct opaque_struct* GTY(()) self_ptr;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) my_ptr global_pointer_var;
extern GTY(()) int_array global_array_var;
extern GTY(()) struct lang_struct* global_lang_struct_var;
extern GTY(()) struct complex_nested global_complex_var;
extern GTY(()) struct chain_node* global_chain_head;
extern GTY(()) struct opaque_struct global_opaque_var;

/* Array of pointers with length option */
struct GTY(()) var_len_struct {
  int count;
  struct my_struct* GTY((length("%h.count"))) items[1];
};

#endif /* TEST_COVERAGE_H */
