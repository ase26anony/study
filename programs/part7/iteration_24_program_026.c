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

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  long field2;
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Skip option */
};

/* TYPE_USER_STRUCT: User-defined marking routines */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int tag;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((tag("0"))) p;  /* Tag option for discrimination */
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_struct_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with desc tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    int ival;
    double dval;
    struct lang_struct* GTY((tag("0"))) child;
  } GTY((tag("1"))) u;
  struct lang_struct* GTY((chain_next)) next;  /* Chain option */
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains array of pointers to unions */
  union my_union* GTY(()) union_array[4];
  
  /* Pointer to callback */
  callback_fn GTY(()) handler;
  
  /* Nested struct */
  struct GTY(()) inner_struct {
    int_array numbers;
    struct complex_nested* GTY((skip)) parent;
  } inner;
  
  /* Chain of structures */
  struct complex_nested* GTY((chain_next)) chain_next;
  struct complex_nested* GTY((chain_prev)) chain_prev;
  
  /* Variable length array with length option */
  struct my_struct** GTY((length("vla_length"))) vla;
  int vla_length;
};

/* Now define the previously opaque struct to complete TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY(()) data;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) struct complex_nested* global_complex;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct opaque_struct global_opaque;

/* Variable-length structure for length option testing */
struct GTY(()) var_len_struct {
  int count;
  struct my_struct* GTY((length("count"))) items[1];
};

/* Another structure with skip option */
struct GTY(()) skip_test {
  void* GTY((skip)) skipped_ptr;
  int* GTY(()) tracked_ptr;
};

#endif /* GTYPE_COVERAGE_TEST_H */
