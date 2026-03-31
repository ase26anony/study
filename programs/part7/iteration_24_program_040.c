/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_struct;  /* TYPE_UNDEFINED when first encountered */

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
  void* GTY((skip)) field2;  /* Using skip option */
};

/* TYPE_USER_STRUCT: User-defined marking routines */
struct GTY((user)) user_struct {
  void* data;
  struct user_struct* GTY((skip)) next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  double d;
  void* p;
  struct my_struct* GTY((tag("1"))) s;
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
  union tree_node* GTY((tag("0"))) u;  /* Using tag option */
  struct lang_struct* GTY((chain_next)) chain;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY((skip)) embedded_struct;  /* TYPE_STRUCT */
  union my_union GTY((skip)) embedded_union;     /* TYPE_UNION */
  my_ptr* GTY((length("array_len"))) ptr_array;  /* TYPE_POINTER in array */
  int_array number_array;                        /* TYPE_ARRAY */
  struct lang_struct* GTY((tag("1"))) lang_ptr;  /* TYPE_LANG_STRUCT */
  callback_fn handler;                           /* TYPE_CALLBACK */
  const char* GTY((skip)) name;                  /* TYPE_STRING */
  int array_len;                                 /* TYPE_SCALAR */
  
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
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct complex_nested* global_complex_list;
extern GTY(()) struct opaque_struct global_opaque;
extern GTY(()) struct lang_struct* global_lang_structs[3];

/* Array with length specifier */
struct GTY(()) array_container {
  int count;
  struct my_struct* GTY((length("count"))) items;
};

/* Another complex type mixing different kinds */
union GTY(()) mixed_union {
  struct my_struct GTY((skip)) s;
  struct lang_struct* GTY((tag("1"))) l;
  callback_fn GTY((skip)) f;
  const char* GTY((skip)) str;
};

#endif /* TEST_COVERAGE_H */
