/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;
typedef void (*callback_func)(void *);

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Callback function pointer */
typedef void (* GTY((callback)) my_callback)(struct tree *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  struct tree * GTY((tag ("0"))) tree_ptr;  /* TYPE_POINTER */
  int GTY((skip)) data;  /* TYPE_SCALAR */
  gty_string GTY((skip)) name;  /* TYPE_STRING */
  my_callback GTY((skip)) cb;  /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  tree_code code;  /* TYPE_SCALAR */
  struct list * GTY((length ("%h.list_count"))) children[10];  /* TYPE_ARRAY */
  int list_count;
  union node_ptr * GTY((skip)) node;  /* TYPE_POINTER */
  const char * GTY((string)) str;  /* TYPE_STRING */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;
};

/* TYPE_ARRAY: Structure with variable-length array */
struct GTY(()) array_container {
  int count;
  struct tree * GTY((length ("%h.count"))) elements[1];  /* TYPE_ARRAY (variable length) */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct list * GTY(()) list_ptr;

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_struct {
  struct list * GTY((chain_next ("%h.next"))) first;  /* TYPE_POINTER */
  union node_ptr GTY((skip)) choice;  /* TYPE_UNION */
  struct array_container * GTY((skip)) array;  /* TYPE_POINTER */
  tree_code codes[5];  /* TYPE_SCALAR array */
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
private:
  struct tree * GTY((skip)) decl_tree;  /* TYPE_POINTER */
  struct list * GTY((skip)) decl_list;  /* TYPE_POINTER */
  
public:
  declaration() : decl_tree(0), decl_list(0) {}
  void set_tree(struct tree *t) { decl_tree = t; }
};
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct complex_struct * GTY((skip)) complex;  /* TYPE_POINTER */
  struct array_container * GTY((skip)) arrays[3];  /* TYPE_ARRAY */
  list_ptr GTY((skip)) list_pointer;  /* TYPE_POINTER (via typedef) */
  #ifdef __cplusplus
  class declaration * GTY((skip)) decl;  /* TYPE_LANG_STRUCT */
  #endif
  gty_string GTY((skip)) description;  /* TYPE_STRING */
};

#endif /* TEST_GTY_INPUT_H */
