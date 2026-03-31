#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY((tag("undefined")));

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_container {
  char * GTY((length("strlen($1) + 1"))) data;
  int length;
} GTY((tag("string_container")));

/* TYPE_STRUCT: Standard C structs */
struct base_struct {
  my_scalar_t value;
  struct base_struct *next;
} GTY((tag("base_struct")));

/* Nested struct for complexity */
struct outer_struct {
  struct base_struct inner GTY((skip));
  int count;
  struct {
    float x;
    float y;
  } point;
} GTY((tag("outer_struct")));

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user)) {
  void *data;
  int size;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union {
  int int_val;
  float float_val;
  char * GTY((length("strlen($1) + 1"))) str_val;
  struct base_struct *struct_ptr;
} GTY((tag("data_union")));

/* TYPE_POINTER: Complex pointer relationships */
struct pointer_network {
  struct pointer_network *self_ptr;
  struct pointer_network *next;
  struct pointer_network *prev;
  struct base_struct **array_of_ptrs;
  void *opaque;
} GTY((tag("pointer_network")));

/* TYPE_ARRAY: Various array types */
struct array_container {
  int fixed_array[10];
  struct base_struct * GTY((length("$1.dynamic_count"))) dynamic_array[];
  int zero_length_array[0];
  int variable_len GTY((length("$1.vlen")));
  struct base_struct *ptr_array[5];
} GTY((tag("array_container")));

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void *) GTY((callback));

struct callback_container {
  callback_func_t handler;
  void * GTY((skip)) user_data;
} GTY((tag("callback_container")));

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((tag("lang_specific"), lang_struct (1))) {
  int lang_field;
  void *lang_data;
};

/* Circular reference for complexity */
struct node_a;
struct node_b;

struct node_a {
  struct node_b *link;
  int id;
} GTY((tag("node_a")));

struct node_b {
  struct node_a *link;
  int id;
} GTY((tag("node_b")));

/* Another forward declaration (TYPE_UNDEFINED if never defined) */
struct never_defined GTY((tag("never_defined")));

#endif /* TEST_GENGYPE_H */
