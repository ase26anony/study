/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct my_struct;
union my_union;

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
  
  /* Pointer to self */
  struct pointer_container* GTY((skip)) self_ptr;
  
  /* Void pointer */
  void* GTY((skip)) generic_ptr;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array (requires length callback) */
  char* GTY((length("strlen($)"))) variable_arr;
  
  /* Array of pointers */
  struct my_struct* GTY((skip)) GTY((length("5"))) ptr_arr[5];
};

/* TYPE_SCALAR: Struct with scalar types */
struct scalar_container GTY(()) {
  long GTY((skip)) counter;
  unsigned GTY((skip)) flags;
  size_t GTY((skip)) size;
  enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: Struct with string fields */
struct string_container GTY(()) {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_str;
  const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

/* Struct using callback type */
struct callback_container GTY(()) {
  callback_fn GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Complex nested structure to test type graph traversal */
struct nested_container GTY(()) {
  struct my_struct base;
  union my_union variant;
  struct pointer_container* GTY((skip)) ptrs;
  struct array_container arrays;
  struct scalar_container scalars;
  struct string_container strings;
  struct callback_container callbacks;
};

/* Template-like macro to generate multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Language-specific structure (simulating Tree nodes) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
  int decl_uid;
  const char* GTY((skip)) decl_name;
  struct lang_struct* GTY((skip)) chain;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
