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
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* p;
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union {
    int ival;
    float fval;
    struct lang_struct* GTY((tag("1"))) child;
  } GTY((tag("0"))) u;
  struct lang_struct* GTY((chain_next)) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* TYPE_STRUCT nested */
  struct my_struct nested_struct;
  
  /* TYPE_UNION nested */
  union my_union nested_union;
  
  /* TYPE_ARRAY of pointers */
  struct my_struct* GTY((length("array_len"))) ptr_array[5];
  int array_len;
  
  /* TYPE_ARRAY of arrays */
  int GTY(()) matrix[3][4];
  
  /* TYPE_POINTER to union */
  union my_union* union_ptr;
  
  /* TYPE_POINTER to callback */
  callback_fn callback_ptr;
  
  /* Chain of structures */
  struct container* GTY((chain_next)) next;
  struct container* GTY((chain_prev)) prev;
  
  /* String array */
  const char* GTY(()) strings[3];
  
  /* Scalar field */
  GTY(()) long scalar_field;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container global_container;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) my_ptr global_my_ptr;

/* Now define the previously opaque struct to resolve TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int resolved;
  struct container* GTY((skip("if (0) skip_this"))) cont;
};

/* Array of various types */
typedef struct GTY(()) variant_array {
  struct my_struct s;
  union my_union u;
  int GTY(()) arr[5];
} variant_array;

extern GTY(()) variant_array global_variants[2];

/* Union containing array */
union GTY(()) union_with_array {
  struct my_struct s;
  int GTY(()) arr[8];
};

/* Struct with callback field */
struct GTY(()) struct_with_callback {
  callback_fn handler;
  void* GTY((skip)) data;
};

#endif /* TEST_COVERAGE_H */
