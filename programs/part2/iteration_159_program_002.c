#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT - Basic struct with GTY annotation */
struct GTY(()) base_struct {
  int x;
  double y;
};

/* TYPE_UNION - Basic union with GTY annotation */
union GTY(()) base_union {
  int as_int;
  double as_double;
  void* as_ptr;
};

/* TYPE_POINTER - Struct containing pointers */
struct GTY(()) pointer_container {
  /* Regular pointer */
  struct base_struct* GTY((skip)) regular_ptr;
  
  /* Pointer to union */
  union base_union* GTY((skip)) union_ptr;
  
  /* Pointer to pointer */
  struct pointer_container** GTY((skip)) double_ptr;
};

/* TYPE_ARRAY - Struct with arrays */
struct GTY(()) array_container {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Array of pointers */
  struct base_struct* GTY((skip)) ptr_arr[5];
  
  /* Multi-dimensional array */
  double GTY((length("3*4"))) matrix[3][4];
};

/* TYPE_SCALAR - Various scalar types with GTY */
struct GTY(()) scalar_container {
  long GTY((skip)) counter;
  unsigned GTY((skip)) flags;
  size_t GTY((skip)) size;
  _Bool GTY((skip)) boolean;
};

/* TYPE_STRING - String types */
struct GTY(()) string_container {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_str;
  const char* GTY((skip)) path;
};

/* TYPE_CALLBACK - Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct GTY(()) callback_container {
  callback_fn GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Template-like macro for generating multiple structs */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct base_struct*);

/* Complex nested structure for type graph */
struct GTY(()) complex_node {
  struct complex_node* GTY((skip)) next;
  struct complex_node* GTY((skip)) prev;
  struct base_struct GTY((skip)) data;
  union base_union GTY((skip)) variant;
};

/* Union containing struct and pointer */
union GTY(()) mixed_union {
  struct base_struct GTY((skip)) as_struct;
  struct pointer_container* GTY((skip)) as_pointer;
  int as_scalar;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_node {
  struct tree_node* GTY((skip)) left;
  struct tree_node* GTY((skip)) right;
  int value;
};

#endif /* TEST_GTY_H */
