/* test-gty-input.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_CODE_ERROR,
  TREE_CODE_VAR,
  TREE_CODE_FUNCTION,
  TREE_CODE_TYPE
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* Forward declarations for mutual dependencies */
struct tree_node;
struct list_node;
union node_union;

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list_node {
  struct list_node * GTY((skip)) next;  /* Skip to avoid infinite recursion */
  struct tree_node * GTY((tag ("0"))) tree_ptr;
  union node_union * GTY((tag ("1"))) union_ptr;
  int GTY((skip)) data;  /* TYPE_SCALAR */
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree_node {
  tree_code code;  /* TYPE_SCALAR */
  gty_string_t name;  /* TYPE_STRING */
  
  /* Fixed-size array of pointers */
  struct list_node * GTY((length ("5"))) children[5];
  
  /* Variable-length array */
  struct tree_node ** GTY((length ("%h.child_count"))) child_ptrs;
  unsigned int child_count;
  
  /* Callback field */
  gty_callback_t callback;  /* TYPE_CALLBACK */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_union {
  int type_tag;  /* Discriminator - TYPE_SCALAR */
  struct tree_node * GTY((tag ("0"))) as_tree;
  struct list_node * GTY((tag ("1"))) as_list;
  gty_string_t GTY((tag ("2"))) as_string;
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree_node * GTY((user)) tree_ptr_t;

/* TYPE_USER_STRUCT: Structure via typedef */
typedef struct GTY((user)) {
  tree_ptr_t root;
  union node_union current;
  gty_string_t filename;
} user_struct_t;

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
class GTY((user)) lang_class {
private:
  struct tree_node * GTY((skip)) m_root;
  user_struct_t * GTY((skip)) m_user_struct;
  
public:
  lang_class() : m_root(0), m_user_struct(0) {}
  void set_root(struct tree_node *root) { m_root = root; }
};
#endif

/* Root structure that ties everything together */
struct GTY((user)) root_container {
  struct list_node *list_head;
  struct tree_node *tree_root;
  union node_union active_node;
  user_struct_t user_data;
  gty_string_t description;
  gty_callback_t notify_callback;
  
#ifdef __cplusplus
  lang_class *lang_obj;
#endif
};

#endif /* TEST_GTY_INPUT_H */
