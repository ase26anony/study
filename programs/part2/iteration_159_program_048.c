#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
  int x;
  double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
  int i;
  double d;
  void* p;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
  /* Pointer to another GTY struct */
  struct my_struct* GTY((skip)) struct_ptr;
  
  /* Pointer to self */
  struct pointer_container* GTY((skip)) self_ptr;
  
  /* Void pointer */
  void* GTY((skip)) generic_ptr;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array (zero-length at end) */
  int GTY((length("n_items"))) var_arr[0];
  int n_items;
};

/* TYPE_SCALAR: Various scalar types */
struct scalar_container GTY(()) {
  long GTY((skip)) counter;
  unsigned GTY((skip)) flags;
  short GTY((skip)) small;
  char GTY((skip)) byte;
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_str;
  const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

struct callback_container GTY(()) {
  callback_fn GTY((skip)) handler;
  void (*GTY((skip)) another_handler)(const char*);
};

/* Complex nested structure for type graph */
struct complex_node GTY(()) {
  struct complex_node* GTY((skip)) next;
  struct complex_node* GTY((skip)) prev;
  union my_union GTY((tag("0"))) data;
  struct array_container arr;
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
  struct recursive_container* GTY((skip)) self;
};

struct forward_decl GTY(()) {
  int value;
  struct recursive_container* GTY((skip)) back_ref;
};

#endif /* TEST_GTY_H */
