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
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Uses forward-declared type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int id;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((skip)) p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_struct_ptr;
typedef union my_union* GTY(()) my_union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
    const char* GTY((tag("2"))) name;
  } u;
  struct lang_struct* GTY((chain_next("%0"))) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY((skip)) embedded_struct;      /* TYPE_STRUCT */
  union my_union GTY((skip)) embedded_union;         /* TYPE_UNION */
  struct lang_struct* GTY((skip)) lang_ptr;          /* TYPE_POINTER to TYPE_LANG_STRUCT */
  int_array numbers;                                 /* TYPE_ARRAY */
  struct complex_nested* GTY((skip)) self_ptr;       /* TYPE_POINTER (recursive) */
  callback_fn handler;                               /* TYPE_CALLBACK */
  const char* GTY((skip)) description;               /* TYPE_STRING */
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) struct complex_nested global_complex;
extern GTY(()) int_array global_int_array;
extern GTY(()) my_struct_ptr global_struct_ptr;

/* Now define the previously forward-declared opaque struct */
struct GTY(()) opaque_struct {
  int magic;
  struct my_struct* GTY((skip)) next;
  struct opaque_struct* GTY((skip)) prev;
};

/* Linked list with chain_next/chain_prev options */
struct GTY(()) linked_node {
  int value;
  struct linked_node* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct linked_node* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) prev;
};

/* Array with length option */
struct GTY(()) variable_array {
  int length;
  int GTY((length("%0.length"))) data[];
};

#endif /* GTYPE_COVERAGE_TEST_H */
