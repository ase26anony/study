/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type as GC root */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type with GTY */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT: Plain C struct with GTY */
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

/* TYPE_UNION: Union type with GTY */
union GTY(()) my_union {
  int i;
  float f;
  void* GTY((skip)) p;
  struct my_struct* GTY((skip)) s;
};

/* TYPE_POINTER: Pointer type definition */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) lang_union {
    int ival;
    double dval;
    struct lang_struct* GTY((tag("1"))) child;
  } GTY((tag("0"))) u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY((skip)) base;          /* TYPE_STRUCT */
  union my_union GTY((skip)) data;            /* TYPE_UNION */
  struct complex_nested* GTY((chain_next)) next;  /* TYPE_POINTER with chain_next */
  struct complex_nested* GTY((chain_prev)) prev;  /* TYPE_POINTER with chain_prev */
  int_array numbers;                          /* TYPE_ARRAY */
  callback_fn handlers[3];                    /* TYPE_CALLBACK array */
  const char* GTY((skip)) name;               /* TYPE_STRING */
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested* global_nested_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct* global_lang_struct;

/* Now define the previously opaque TYPE_UNDEFINED type */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY((skip)) link;
};

/* Struct with length option for variable-sized array */
struct GTY(()) varray_struct {
  int count;
  int GTY((length("%h.count"))) data[1];  /* TYPE_ARRAY with length option */
};

/* Union with nested struct to test deep processing */
union GTY(()) nested_union {
  struct {
    int x;
    int y;
  } GTY((skip)) point;
  struct complex_nested* GTY((skip)) complex;
};

#endif /* TEST_COVERAGE_H */
