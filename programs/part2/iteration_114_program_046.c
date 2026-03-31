#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_container {
  char * GTY((length("strlen($1) + 1"))) data;
  int length;
};

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
  my_scalar_t value;
  struct base_struct *next;
};

/* Nested struct for complexity */
struct outer_struct GTY(()) {
  struct inner_struct GTY(()) {
    int x;
    double y;
  } inner;
  struct base_struct *base_ptr;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user)) {
  void *user_data;
  int user_id;
};

/* TYPE_UNION: Union with GTY members */
union data_union GTY(()) {
  int int_val;
  double double_val;
  char * GTY((tag("0"))) string_val;
  struct base_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(()) {
  struct undefined_struct *undefined_ptr;  /* TYPE_UNDEFINED pointer */
  struct base_struct **double_ptr;
  void (*func_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
  int fixed_array[10];
  struct base_struct * GTY((length("$1.dynamic_count"))) dynamic_array[];
  int dynamic_count;
  
  /* Zero-length array */
  char zero_length_array[0];
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void *) GTY((callback));

struct callback_container GTY(()) {
  callback_func handler;
  void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1))) {
  int lang_field;
  void *lang_data;
};

/* Circular reference for complexity */
struct node_a GTY(()) {
  struct node_b *b_ptr;
  int value;
};

struct node_b GTY(()) {
  struct node_a *a_ptr;
  struct node_a array_of_a[5];
  double data;
};

/* Complete the undefined struct definition */
struct undefined_struct GTY(()) {
  int finally_defined;
  struct base_struct *link;
};

/* Function pointer typedefs */
typedef int (*simple_func_ptr)(void) GTY(());

#endif /* TEST_GENGYPE_H */
