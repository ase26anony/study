/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
  int GTY((skip)) x;
  float GTY((skip)) y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union GTY(()) base_union {
  int GTY((skip)) int_val;
  float GTY((skip)) float_val;
  void* GTY((skip)) ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_container {
  /* Pointer to another GTY-annotated struct */
  struct base_struct* GTY((skip)) struct_ptr;
  
  /* Pointer to self (recursive type) */
  struct pointer_container* GTY((skip)) next;
  
  /* Void pointer */
  void* GTY((skip)) data;
};

/* TYPE_ARRAY: Struct with arrays */
struct GTY(()) array_container {
  /* Fixed-size array */
  int GTY((length("10"))) fixed_array[10];
  
  /* Array of pointers */
  struct base_struct* GTY((skip)) ptr_array[5];
};

/* TYPE_SCALAR: Various scalar types with GTY */
struct GTY(()) scalar_container {
  long GTY((skip)) counter;
  unsigned GTY((skip)) flags;
  double GTY((skip)) value;
  enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
  const char* GTY((skip)) name;
  char* GTY((skip)) buffer;
  const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

struct GTY(()) callback_container {
  callback_fn GTY((skip)) handler;
  void (* GTY((skip)) another_handler)(const char*);
};

/* Complex nested type for TYPE_STRUCT recursion */
struct GTY(()) complex_node {
  int GTY((skip)) id;
  struct complex_node* GTY((skip)) left;
  struct complex_node* GTY((skip)) right;
  union base_union GTY((skip)) data;
};

/* Template-like macro to generate multiple structs */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(float);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;
struct GTY(()) another_struct;

struct GTY(()) forward_decl_struct {
  struct another_struct* GTY((skip)) link;
};

struct GTY(()) another_struct {
  struct forward_decl_struct* GTY((skip)) backlink;
  int GTY((skip)) value;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific_struct {
  int GTY((skip)) decl_uid;
  const char* GTY((skip)) decl_name;
  struct lang_specific_struct* GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
