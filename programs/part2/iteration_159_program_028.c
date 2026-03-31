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
  struct base_struct* GTY((skip)) struct_ptr;
  union base_union* GTY((skip)) union_ptr;
  void* GTY((skip)) opaque_ptr;
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct GTY(()) array_container {
  int GTY((length("10"))) fixed_arr[10];
  struct base_struct* GTY((length("5"))) struct_arr[5];
};

/* TYPE_SCALAR: Direct scalar type annotation */
long GTY((skip)) global_counter;

/* TYPE_STRING: String field */
struct GTY(()) string_container {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_str;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct GTY(()) callback_container {
  callback_fn GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Complex nested type for recursive graph processing */
struct GTY(()) complex_node {
  struct complex_node* GTY((skip)) next;
  struct complex_node* GTY((skip)) prev;
  union base_union GTY((skip)) data;
  struct array_container GTY((skip)) arrays;
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(float);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;
struct GTY(()) another_struct;

struct GTY(()) forward_decl_struct {
  struct another_struct* GTY((skip)) link;
  int value;
};

struct GTY(()) another_struct {
  struct forward_decl_struct* GTY((skip)) backlink;
  float data;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific_node {
  int decl_uid;
  const char* GTY((skip)) name;
  struct lang_specific_node* GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
