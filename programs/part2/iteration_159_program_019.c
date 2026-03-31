#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
  int x;
  float y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union GTY(()) base_union {
  int as_int;
  float as_float;
  void* as_ptr;
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_container {
  /* Regular pointer */
  struct base_struct* GTY((skip)) regular_ptr;
  
  /* Pointer with length specifier */
  int* GTY((length("len"))) variable_len_ptr;
  unsigned len;
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array */
  char GTY((length("str_len"))) variable_arr[1];
  unsigned str_len;
  
  /* Array of pointers */
  struct base_struct* GTY((skip)) ptr_array[5];
};

/* TYPE_SCALAR: Direct scalar types with GTY */
typedef long GTY((skip)) long_type;
typedef unsigned GTY((skip)) bitmask_type;

/* TYPE_STRING: String types */
struct GTY(()) string_container {
  const char* GTY((skip)) constant_string;
  char* GTY((skip)) mutable_string;
  const char* GTY((length("strlen(name)"))) name;
};

/* TYPE_CALLBACK: Callback function types */
typedef void (*simple_callback)(int) GTY((callback));
typedef int (*complex_callback)(struct base_struct*, void*) GTY((callback));

/* Complex nested structure for type graph */
struct GTY(()) complex_node {
  struct complex_node* GTY((skip)) next;
  struct complex_node* GTY((skip)) prev;
  union base_union GTY((tag("type"))) data;
  int node_type;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(float);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
  struct tree_node* GTY((skip)) left;
  struct tree_node* GTY((skip)) right;
  int value;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific {
  int decl_type;
  const char* GTY((skip)) name;
  struct lang_specific* GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
