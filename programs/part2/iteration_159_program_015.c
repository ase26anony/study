/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct my_struct;
union my_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
  int id;
  const char * GTY((skip)) name;  /* TYPE_STRING */
  long GTY((skip)) counter;       /* TYPE_SCALAR */
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
  int int_val;
  float float_val;
  struct my_struct * GTY((skip)) struct_ptr;  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct array_container GTY(()) {
  int GTY((length("10"))) arr[10];  /* TYPE_ARRAY */
  struct my_struct * GTY((skip)) next;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

/* Struct using callback type */
struct callback_container GTY(()) {
  callback_fn handler;
  int data;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Instantiate template-like structs */
DEF_PAIR(int);
DEF_PAIR(double);

/* Complex nested type graph */
struct graph_node GTY(()) {
  int value;
  struct graph_node * GTY((skip)) left;
  struct graph_node * GTY((skip)) right;
  union my_union data;
};

/* Language-specific structure hook (simulating Tree nodes) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
  int decl_uid;
  const char * GTY((skip)) decl_name;
  struct lang_struct * GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
