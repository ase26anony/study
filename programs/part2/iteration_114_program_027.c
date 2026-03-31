#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_UNDEFINED: Forward declaration that will remain undefined */
struct undefined_struct GTY(());
typedef struct undefined_struct *undefined_ptr_t GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());
typedef char char_scalar_t GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_container {
  char * GTY((length("str_len"))) string_field;
  int str_len;
} GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
  my_scalar_t scalar_field;
  struct base_struct *next GTY(());
  int data;
};

/* Nested struct for complexity */
struct outer_struct GTY(()) {
  struct base_struct inner GTY(());
  int outer_data;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
  int user_data;
  void *user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(()) {
  int int_val;
  char * GTY((tag("0"))) str_val;
  struct base_struct * GTY((tag("1"))) struct_ptr;
  double double_val;
};

/* TYPE_POINTER: Various pointer types creating a graph */
struct pointer_network GTY(()) {
  struct base_struct *direct_ptr GTY(());
  struct pointer_network *self_ptr GTY(());
  struct undefined_struct *forward_ptr GTY(());
  union data_union *union_ptr GTY(());
};

/* TYPE_ARRAY: Arrays with different attributes */
struct array_container GTY(()) {
  /* Fixed-size array */
  int fixed_array[10] GTY(());
  
  /* Zero-length array */
  char zero_array[0] GTY(());
  
  /* Array with length attribute */
  struct base_struct * GTY((length("array_len"))) dyn_array;
  int array_len;
  
  /* Nested array */
  int matrix[5][5] GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY(()) {
  callback_func_t handler GTY(());
  void (*regular_func_ptr)(void);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
  int lang_data;
  void *lang_private;
};

/* Complex nested type with all kinds */
struct master_container GTY(()) {
  /* SCALAR */
  my_scalar_t master_scalar;
  
  /* STRUCT */
  struct base_struct nested_struct GTY(());
  
  /* UNION */
  union data_union data GTY(());
  
  /* POINTER */
  struct pointer_network *network GTY(());
  
  /* ARRAY */
  struct array_container arrays GTY(());
  
  /* STRING */
  char * GTY((length("master_str_len"))) master_string;
  int master_str_len;
  
  /* CALLBACK */
  callback_func_t master_callback GTY(());
  
  /* USER STRUCT */
  struct user_handled_struct user GTY(());
  
  /* LANG STRUCT */
  struct lang_specific_struct lang GTY(());
  
  /* Forward pointer for undefined type */
  undefined_ptr_t undefined_ref GTY(());
};

/* Circular reference for pointer testing */
struct circular_a GTY(()) {
  struct circular_b *link GTY(());
  int value;
};

struct circular_b GTY(()) {
  struct circular_a *link GTY(());
  int value;
};

#endif /* TEST_GENGTYPE_H */
