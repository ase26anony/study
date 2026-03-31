/* Test header for gengtype coverage of type state writing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_A,
  CODE_B,
  CODE_C
} tree_code GTY(());

/* TYPE_STRING: String type */
typedef const char * GTY((string)) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) callback_func)(void *data);

/* TYPE_STRUCT: Basic structure */
struct GTY(()) base_struct {
  int scalar_field;           /* TYPE_SCALAR */
  string_type name;           /* TYPE_STRING */
  callback_func callback;     /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) {
  struct base_struct * GTY((skip)) ptr;  /* TYPE_POINTER */
  int count;
} user_struct;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array */
  struct base_struct * GTY((length ("5"))) fixed_array[5];
  
  /* Variable-length array */
  struct base_struct ** GTY((variable_length)) var_array;
  int var_length;
  
  /* Array of scalars */
  int scalar_array[10];
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) discriminated_union {
  int type;
  struct base_struct * GTY((tag ("0"))) as_base;
  user_struct * GTY((tag ("1"))) as_user;
  struct array_container * GTY((tag ("2"))) as_array;
};

/* TYPE_POINTER: Pointer-only structure */
struct GTY(()) pointer_network {
  struct pointer_network *next;      /* TYPE_POINTER */
  struct pointer_network *prev;      /* TYPE_POINTER */
  struct base_struct *data;          /* TYPE_POINTER */
  user_struct *user_data;            /* TYPE_POINTER */
};

/* Linked list structure with chain_next */
struct GTY((chain_next ("%h.next"))) linked_list {
  struct linked_list *next;
  struct discriminated_union data;
  int id;
};

/* Nested structure for complex dependencies */
struct GTY(()) complex_node {
  union discriminated_union choice;
  struct array_container arrays;
  struct pointer_network *network;
  struct linked_list *list;
  tree_code code;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
class GTY(()) lang_class {
private:
  struct complex_node *node;
  string_type class_name;
  
public:
  lang_class() : node(NULL), class_name("default") {}
  void set_node(struct complex_node *n) { node = n; }
};
#endif

/* Root container that references everything */
struct GTY(()) root_container {
  struct base_struct base;
  user_struct user;
  struct array_container arrays;
  union discriminated_union union_field;
  struct pointer_network *network_root;
  struct linked_list *list_head;
  struct complex_node *complex_tree;
#ifdef __cplusplus
  class lang_class *lang_obj;
#endif
  tree_code root_code;
  string_type root_name;
  callback_func root_callback;
};

#endif /* TEST_GTY_INPUT_H */
