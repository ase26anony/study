/* Test header to cover all gengtype-state.cc switch cases */
#ifndef GTYPE_TEST_COVERAGE_H
#define GTYPE_TEST_COVERAGE_H

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
  struct opaque_struct* GTY(()) opaque_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;  /* skip option indicates user handles marking */
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* GTY(()) p;
  struct my_struct* GTY(()) s;
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
  struct lang_struct* GTY((chain_next)) next;
};

/* Now define the previously undefined type to complete TYPE_UNDEFINED -> TYPE_STRUCT */
struct opaque_struct {
  int id;
  struct my_struct* GTY(()) data;
  struct opaque_struct* GTY((chain_next)) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains multiple type kinds */
  struct my_struct GTY(()) base;          /* TYPE_STRUCT */
  union my_union GTY(()) choice;          /* TYPE_UNION */
  my_ptr GTY(()) ptr;                     /* TYPE_POINTER */
  int_array GTY(()) numbers;              /* TYPE_ARRAY */
  struct lang_struct* GTY(()) lang_node;  /* TYPE_LANG_STRUCT */
  callback_fn GTY(()) handler;            /* TYPE_CALLBACK */
  const char* GTY(()) name;               /* TYPE_STRING */
  struct user_struct GTY(()) user_data;   /* TYPE_USER_STRUCT */
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested global_complex;
extern GTY(()) struct opaque_struct* GTY(()) global_opaque_list;
extern GTY(()) struct lang_struct* GTY(()) global_lang_tree;

/* Array of pointers with length field */
struct GTY(()) variable_array_container {
  int count;
  struct my_struct* GTY((length("%h.count"))) items[1];
};

/* Chain-linked list structure */
struct GTY(()) chain_node {
  int value;
  struct chain_node* GTY((chain_next)) next;
  struct chain_node* GTY((chain_prev)) prev;
};

#endif /* GTYPE_TEST_COVERAGE_H */
