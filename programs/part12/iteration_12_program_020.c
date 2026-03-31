/* Test header for gengtype coverage of type state writing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_STRUCT: Regular structure with GC-tagged pointers */
struct GTY(()) base_struct {
  int id;
  const char * GTY((skip)) name;  /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct * GTY((tag("0"))) base_ptr;  /* TYPE_POINTER */
  int value;
} my_user_struct_t;

/* TYPE_UNION: Discriminated union with pointers */
union GTY((desc ("%0.type"))) data_union {
  struct base_struct * GTY((tag("1"))) ptr_base;  /* TYPE_POINTER */
  my_user_struct_t * GTY((tag("2"))) ptr_user;    /* TYPE_POINTER */
  int type;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  struct base_struct * GTY((length ("10"))) fixed_array[10];
  
  /* Variable-length array */
  my_user_struct_t * GTY((variable_length)) var_array;
  int var_length;
  
  /* Nested array */
  union data_union GTY((length ("5"))) union_array[5];
};

/* Linked list structure for chain_next/chain_prev */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
  struct linked_node *next;
  struct linked_node *prev;
  struct base_struct *data;
  int index;
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct array_container * GTY(()) container_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) event_callback)(void * GTY((skip)) data);

/* Structure using callback */
struct GTY(()) event_handler {
  event_callback callback;
  void * GTY((skip)) user_data;
  struct event_handler *next;
};

/* TYPE_SCALAR: Enumeration type */
enum node_type {
  NODE_BASE,
  NODE_USER,
  NODE_ARRAY,
  NODE_LINKED
};

/* Structure with scalar enum */
struct GTY(()) typed_node {
  enum node_type type;
  union data_union data;
  struct typed_node *children[4];  /* TYPE_ARRAY */
};

/* TYPE_STRING: Explicit string type */
struct GTY(()) string_container {
  const char * GTY((string)) text;
  int length;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) root_container {
  struct base_struct root;
  my_user_struct_t user;
  union data_union data;
  struct array_container arrays;
  struct linked_node *list_head;
  container_ptr_t container;
  struct event_handler *handlers;
  struct typed_node *node_tree;
  struct string_container strings[3];
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;

/* Self-referential structure */
struct GTY(()) tree_node {
  int id;
  const char * GTY((string)) label;
  struct tree_node * GTY((length ("%h.child_count"))) children;
  struct tree_node *parent;
  int child_count;
};

/* Union with self-reference */
union GTY((desc ("%0.node_type"))) recursive_union {
  struct tree_node * GTY((tag("0"))) tree;
  struct linked_node * GTY((tag("1"))) list;
  int node_type;
};

#endif /* TEST_GTY_INPUT_H */
