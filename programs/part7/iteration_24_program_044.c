/* Test header to cover all TYPE_* cases in gengtype-state.cc */

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

/* TYPE_STRUCT: Plain C structure */
struct GTY(()) my_struct {
  int field1;
  struct opaque_struct* GTY(()) opaque_ptr;  /* Pointer to undefined type */
  callback_fn GTY(()) handler;               /* Callback type */
};

/* TYPE_USER_STRUCT: User-defined marking routine */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;  /* skip option */
  struct my_struct* GTY((chain_next("next"))) next;  /* chain_next option */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((tag("0"))) p;  /* tag option for discriminant */
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
extern GTY(()) my_ptr global_pointer;

/* TYPE_LANG_STRUCT: Language-specific structure with desc tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union my_union GTY((tag("0"))) u;  /* Nested union with tag */
  struct lang_struct* GTY((chain_next("next"))) next;
};

/* Complex nesting to ensure deep traversal */
struct GTY(()) container_struct {
  /* Array of structs */
  struct my_struct GTY(()) items[5];
  
  /* Pointer to union */
  union my_union* GTY(()) union_ptr;
  
  /* User struct with chain */
  struct user_struct* GTY(()) user_chain;
  
  /* Language struct */
  struct lang_struct GTY(()) lang_node;
  
  /* String array */
  const char* GTY(()) names[3];
  
  /* Callback array */
  callback_fn GTY(()) handlers[2];
  
  /* Multi-dimensional array */
  int GTY(()) matrix[4][4];
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct container_struct global_container;
extern GTY(()) union my_union global_union;
extern GTY(()) struct user_struct* global_user_list;
extern GTY(()) struct lang_struct* global_lang_tree;

/* TYPE_UNDEFINED: Now define the previously forward-declared struct */
struct GTY(()) opaque_struct {
  int id;
  struct container_struct* GTY(()) container;
};

/* Additional complex type with length option */
struct GTY(()) variable_length_struct {
  int count;
  int GTY((length("%0.count"))) data[];  /* Variable length array */
};

#endif /* TEST_COVERAGE_H */
