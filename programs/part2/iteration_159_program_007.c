/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
  int x;
  double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
  int int_val;
  double double_val;
  void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
  /* Pointer to another GTY-annotated struct */
  struct my_struct* GTY((skip)) struct_ptr;
  
  /* Pointer to self for type graph complexity */
  struct pointer_container* GTY((skip)) next;
  
  /* Void pointer */
  void* GTY((skip)) data;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array (requires length callback) */
  int GTY((length("var_len"))) *var_arr;
  size_t var_len;
};

/* TYPE_SCALAR: Direct scalar types with GTY */
struct scalar_container GTY(()) {
  long GTY((skip)) counter;
  unsigned GTY((skip)) flags;
  enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_str;
  const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
  callback_fn GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Complex nested type for type graph testing */
struct complex_node GTY(()) {
  int id;
  struct complex_node* GTY((skip)) left;
  struct complex_node* GTY((skip)) right;
  union my_union GTY((tag("type"))) data;
  int type;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for mutual recursion */
struct forward_decl;
struct recursive_container GTY(()) {
  struct forward_decl* GTY((skip)) fwd_ptr;
};

struct forward_decl GTY(()) {
  int value;
  struct recursive_container* GTY((skip)) container;
};

#endif /* TEST_GTY_H */
