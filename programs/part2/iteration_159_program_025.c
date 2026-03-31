/* test-gty.h - Header file with various GTY annotations */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

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
  
  /* Variable-length array (pointer representation) */
  int* GTY((length("dynamic_len"))) dynamic_arr;
  int dynamic_len;
};

/* TYPE_SCALAR: Direct scalar type annotation */
long GTY((skip)) global_counter;

/* TYPE_STRING: String types */
struct string_container GTY(()) {
  const char* GTY((skip)) name;
  char* GTY((skip)) buffer;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

struct callback_container GTY(()) {
  callback_fn handler;
  int data;
};

/* Complex nested structure for deep type graph */
struct nested_struct GTY(()) {
  struct my_struct inner;
  union my_union choice;
  struct pointer_container* GTY((skip)) next;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for mutual reference */
struct forward_declared GTY(());

struct mutual_ref_a GTY(()) {
  struct mutual_ref_b* GTY((skip)) b_ptr;
  int value_a;
};

struct mutual_ref_b GTY(()) {
  struct mutual_ref_a* GTY((skip)) a_ptr;
  int value_b;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
