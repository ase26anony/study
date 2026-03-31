/* Test header to cover all gengtype-state.cc switch cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration to create undefined type reference */
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
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* GTY((skip)) p;
  struct my_struct* GTY((skip)) s;
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

/* Now define the previously opaque struct to resolve TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int id;
  struct my_struct* GTY((skip)) link;
  union my_union data;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* TYPE_STRUCT */
  struct my_struct embedded_struct;
  
  /* TYPE_UNION */
  union my_union embedded_union;
  
  /* TYPE_POINTER */
  struct opaque_struct* GTY((skip)) ptr_to_opaque;
  
  /* TYPE_ARRAY */
  int GTY(()) number_array[20];
  
  /* TYPE_ARRAY of pointers */
  struct my_struct* GTY(()) ptr_array[15];
  
  /* TYPE_ARRAY of arrays */
  int GTY(()) matrix[10][10];
  
  /* TYPE_LANG_STRUCT */
  struct lang_struct* GTY((skip)) lang_node;
  
  /* TYPE_STRING */
  const char* GTY((skip)) name;
  
  /* TYPE_CALLBACK */
  callback_fn handler;
  
  /* Chain for linked list */
  struct complex_nested* GTY((chain_next)) next;
  struct complex_nested* GTY((chain_prev)) prev;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct opaque_struct global_opaque_var;
extern GTY(()) struct complex_nested global_complex_var;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct global_lang_struct;

/* Variable-length array structure */
struct GTY(()) var_len_struct {
  int length;
  int GTY((length("%0.length"))) data[];
};

/* Union with variable-length array */
union GTY(()) var_len_union {
  int type;
  struct var_len_struct* GTY((skip)) vls;
};

#endif /* TEST_COVERAGE_H */
