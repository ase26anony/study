/* Test header to cover all gengtype-state.cc switch cases */
#ifndef TEST_GTY_COVERAGE_H
#define TEST_GTY_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  long field2;
};

/* TYPE_USER_STRUCT: User-defined marking routines */
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

/* TYPE_POINTER: Pointer type definitions */
typedef struct my_struct* GTY(()) my_struct_ptr;
typedef union my_union* GTY(()) my_union_ptr;

/* TYPE_ARRAY: Fixed-size array types */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    struct my_struct* GTY((tag("0"))) s;
    union my_union* GTY((tag("1"))) u;
  } GTY((tag("code"))) u;
};

/* TYPE_SCALAR: Global scalar variables */
extern GTY(()) int global_scalar_int;
extern GTY(()) long global_scalar_long;

/* TYPE_STRING: String types */
extern GTY(()) const char* global_string;
extern GTY(()) char* GTY((length("strlen(%h)"))) dynamic_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
typedef int (* GTY(()) callback_with_arg)(int, void*);

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Chain of structures */
  struct complex_nested* GTY((chain_next("%h.next"))) next;
  struct complex_nested* GTY((chain_prev("%h.prev"))) prev;
  
  /* Array of pointers */
  struct my_struct* GTY(()) struct_array[4];
  
  /* Union field */
  union my_union data_union;
  
  /* Pointer to array */
  int_array* GTY(()) array_ptr;
  
  /* Callback */
  callback_fn GTY(()) handler;
  
  /* String */
  const char* GTY(()) name;
  
  /* Scalar */
  int id;
  
  /* Pointer to lang_struct */
  struct lang_struct* GTY(()) lang_node;
};

/* TYPE_STRUCT with skip option */
struct GTY(()) skip_struct {
  int visible_field;
  void* GTY((skip)) hidden_pointer;
  int GTY((skip)) hidden_counter;
};

/* Union with tag */
union GTY((tag("TYPE_TAG"))) tagged_union {
  int GTY((tag("0"))) as_int;
  void* GTY((tag("1"))) as_ptr;
  struct my_struct* GTY((tag("2"))) as_struct;
};

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY(()) child;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct complex_nested* global_complex_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) callback_fn global_callback;
extern GTY(()) struct opaque_struct global_opaque;

/* Structure with variable-length array */
struct GTY(()) var_len_struct {
  int count;
  struct my_struct* GTY((length("%h.count"))) items[1];
};

/* Another complex nesting example */
struct GTY(()) outer_container {
  /* Direct struct */
  struct my_struct direct;
  
  /* Pointer to union */
  union my_union* GTY(()) union_ptr;
  
  /* Array of structs */
  struct skip_struct skipped[3];
  
  /* Nested anonymous union */
  union {
    int x;
    struct my_struct* GTY(()) y;
  } GTY(()) anonymous_union;
  
  /* Pointer to callback */
  callback_with_arg GTY(()) processor;
};

#endif /* TEST_GTY_COVERAGE_H */
