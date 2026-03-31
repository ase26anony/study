/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

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
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    int ival;
    double dval;
    struct lang_struct* GTY((tag("0"))) child;
  } u;
  struct lang_struct* GTY((chain_next)) next;  /* Using chain_next */
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY(()) nested_struct;      /* TYPE_STRUCT */
  union my_union GTY(()) nested_union;         /* TYPE_UNION */
  struct my_struct* GTY(()) nested_ptr;        /* TYPE_POINTER */
  int GTY(()) nested_array[20];                /* TYPE_ARRAY */
  const char* GTY(()) nested_string;           /* TYPE_STRING */
  callback_fn nested_callback;                 /* TYPE_CALLBACK */
  struct lang_struct* GTY(()) nested_lang;     /* TYPE_LANG_STRUCT */
  
  /* Chain for linked list */
  struct complex_nested* GTY((chain_next)) next;
  struct complex_nested* GTY((chain_prev)) prev;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct complex_nested global_complex;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct_ptr_array global_struct_ptr_array;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_field;
  struct opaque_struct* GTY(()) next;
};

/* Array with length option */
struct GTY(()) variable_length {
  int count;
  int GTY((length("%0.count"))) data[];
};

#endif /* TEST_COVERAGE_H */
