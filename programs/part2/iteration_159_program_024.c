/* test-gty.h - Header file with GTY annotations for gengtype testing */

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
  int i;
  double d;
  void* p;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
  /* Regular pointer */
  struct my_struct* GTY((skip)) regular_ptr;
  
  /* Pointer with length attribute */
  int* GTY((length("len"))) array_ptr;
  unsigned len;
};

/* TYPE_ARRAY: Struct with various array types */
struct array_container GTY(()) {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_arr[10];
  
  /* Variable-length array with pointer */
  char* GTY((length("str_len"))) string_arr;
  int str_len;
  
  /* Nested array in struct */
  struct {
    float GTY((length("5"))) nested_arr[5];
  } nested;
};

/* TYPE_SCALAR: Struct with scalar types */
struct scalar_container GTY(()) {
  long GTY((skip)) counter;
  unsigned long GTY((skip)) flags;
  short GTY((skip)) small;
};

/* TYPE_STRING: Struct with string fields */
struct string_container GTY(()) {
  const char* GTY((skip)) name;
  char* GTY((skip)) mutable_name;
  const char* GTY((length("desc_len"))) description;
  int desc_len;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

struct callback_container GTY(()) {
  callback_fn GTY((skip)) handler;
  void* GTY((skip)) user_data;
};

/* Complex nested type for TYPE_GRAPH testing */
struct node GTY(()) {
  int value;
  struct node* GTY((skip)) next;
  struct node* GTY((skip)) prev;
};

struct graph GTY(()) {
  struct node* GTY((skip)) nodes;
  unsigned node_count;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct node*);

/* Forward declaration for mutual recursion */
struct forward_decl GTY(());

struct recursive_container GTY(()) {
  struct forward_decl* GTY((skip)) fwd_ptr;
  struct recursive_container* GTY((skip)) self_ptr;
};

struct forward_decl GTY(()) {
  int data;
  struct recursive_container* GTY((skip)) container;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
