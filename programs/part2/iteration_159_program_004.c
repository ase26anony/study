/* test-gty.h - Comprehensive GTY test header */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct forward_declared_struct;

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
  /* Pointer to another GTY struct */
  struct my_struct* GTY((skip)) child;
  
  /* Pointer to forward declared struct */
  struct forward_declared_struct* GTY((skip)) forward_ptr;
  
  /* Void pointer */
  void* GTY((skip)) opaque;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array (pointer with length) */
  char* GTY((length("str_len + 1"))) variable_arr;
  int str_len;
};

/* TYPE_SCALAR: Direct scalar types with GTY */
struct scalar_container GTY(()) {
  long GTY((skip)) counter;
  unsigned GTY((skip)) flags;
  size_t GTY((skip)) size;
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_str;
  const char* GTY((skip)) static_string;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
  callback_fn GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Complex nested type for recursive processing */
struct nested_container GTY(()) {
  struct pointer_container* GTY((skip)) ptr_container;
  struct array_container GTY((tag("0"))) arr_container;
  union my_union GTY((tag("1"))) union_member;
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Language-specific structure simulation */
#ifdef __cplusplus
extern "C" {
#endif

/* Simulating tree nodes for TYPE_LANG_STRUCT */
struct tree_common GTY(()) {
  int code;
  union tree_union* GTY((skip)) u;
};

union tree_union GTY(()) {
  struct tree_common GTY((tag("0"))) common;
  struct tree_decl GTY((tag("1"))) decl;
};

struct tree_decl GTY((tag ("TS_VAR_DECL"))) {
  struct tree_common common;
  const char* GTY((skip)) name;
  struct tree_decl* GTY((skip)) chain;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
