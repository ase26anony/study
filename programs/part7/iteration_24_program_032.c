/* Test header to cover all gengtype-state.cc switch cases */
#ifndef GTYPE_COVERAGE_TEST_H
#define GTYPE_COVERAGE_TEST_H

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
  /* User will provide marking routines elsewhere */
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

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;  /* Discriminant */
  union {
    struct my_struct* GTY((tag("0"))) as_struct;
    union my_union* GTY((tag("1"))) as_union;
    int GTY((tag("2"))) as_int;
  } GTY((desc("%0.code"))) u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Chain of structures */
  struct complex_nested* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct complex_nested* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) prev;
  
  /* Array of pointers */
  struct my_struct* GTY(()) ptr_array[5];
  
  /* Union field */
  union my_union data_union;
  
  /* Language structure */
  struct lang_struct lang_data;
  
  /* Callback */
  callback_fn handler;
  
  /* Variable length array (uses length option) */
  int GTY((length("%0.dyn_length"))) *dyn_array;
  int dyn_length;
};

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY((skip)) sibling;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct lang_struct global_lang_struct_var;
extern GTY(()) struct complex_nested* global_complex_ptr;
extern GTY(()) struct user_struct global_user_struct;

/* Array of different pointer types */
typedef void* GTY(()) generic_ptr;
extern GTY(()) generic_ptr ptr_collection[20];

/* Nested array type */
typedef struct my_struct* GTY(()) struct_ptr_array[3][3];
extern GTY(()) struct_ptr_array matrix_of_pointers;

#endif /* GTYPE_COVERAGE_TEST_H */
