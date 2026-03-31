/* Test header for gengtype coverage */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_func)(void *data);

/* Forward declarations */
struct tree_node;
struct list_node;
union node_union;

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list_node {
  struct list_node * GTY((skip)) next;  /* chain_next handles linking */
  struct tree_node * GTY((tag ("0"))) tree_ptr;
  union node_union * GTY((tag ("1"))) union_ptr;
  int data;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree_node {
  /* Fixed-size array of pointers */
  struct list_node * GTY((length ("5"))) children[5];
  
  /* Variable-length array */
  struct tree_node ** GTY((length ("%h.child_count"))) child_ptrs;
  
  /* Array of strings */
  gty_string GTY((length ("%h.name_count"))) names;
  
  int child_count;
  int name_count;
  tree_code code;  /* TYPE_SCALAR */
  gty_string GTY((string)) label;  /* TYPE_STRING */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_union {
  struct tree_node * GTY((tag ("0"))) as_tree;
  struct list_node * GTY((tag ("1"))) as_list;
  int type_tag;
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree_node * GTY((user)) tree_ptr;

/* TYPE_USER_STRUCT: User-defined structure type */
typedef struct GTY((user)) {
  tree_ptr root;
  struct list_node *head;
  gty_callback_func callback;  /* TYPE_CALLBACK */
  int user_data;
} user_container;

/* Nested structure for complex testing */
struct GTY(()) outer_struct {
  /* Nested anonymous union */
  union {
    struct tree_node * GTY((tag ("0"))) tree_member;
    struct list_node * GTY((tag ("1"))) list_member;
  } GTY((desc ("%h.union_type"))) u;
  
  int union_type;
  
  /* Pointer to array */
  struct tree_node ** GTY((length ("%h.array_size"))) node_array;
  int array_size;
  
  /* Chain of structures */
  struct outer_struct * GTY((chain_next ("%h.chain"))) chain;
};

#ifdef __cplusplus

/* TYPE_LANG_STRUCT: C++ class definition */
class GTY((user)) lang_class {
private:
  struct tree_node * GTY((reorder ("resort_gc_cpp_base"))) root_node;
  user_container *container;
  
public:
  lang_class() : root_node(0), container(0) {}
  virtual ~lang_class() {}
  
  void set_root(struct tree_node *node) { root_node = node; }
  struct tree_node *get_root() { return root_node; }
};

/* Another C++ class with inheritance */
class GTY((user)) derived_class : public lang_class {
private:
  struct list_node *list_head;
  
public:
  derived_class() : list_head(0) {}
};

#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_INPUT_H */
