/* GTY type definitions to cover all switch cases in gengtype-state.cc */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_NONE,
  CODE_LIST,
  CODE_TREE
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* Forward declarations */
struct tree;
struct list;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("CODE_TREE"))) ptr_tree;
  struct list * GTY((tag ("CODE_LIST"))) ptr_list;
  int type;
};

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list *next;
  struct list *prev;
  union node_ptr data;
  int GTY((skip)) count;  /* Non-pointer field */
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree {
  /* Fixed-size array of pointers */
  struct list * GTY((length ("10"))) children[10];
  
  /* Variable-length array */
  struct tree ** GTY((length ("%h.child_count"))) more_children;
  
  /* Array of strings */
  gty_string GTY((length ("%h.name_count"))) names;
  
  int child_count;
  int name_count;
  tree_code code;  /* TYPE_SCALAR */
  
  /* Callback field */
  gty_callback callback;
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree * GTY((user)) tree_ptr;

/* TYPE_USER_STRUCT: User-defined structure type */
typedef struct list user_list GTY((user));

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
class GTY((user)) declaration {
  tree_ptr decl_tree;
  user_list *decl_list;
  gty_string name;
public:
  declaration();
  virtual ~declaration();
};
#endif

/* Root structure that references all types */
struct GTY(()) root_container {
  struct list *first_list;
  struct tree *first_tree;
  union node_ptr first_node;
  tree_ptr tree_pointer;
  user_list *user_list_ptr;
  gty_string root_name;
  gty_callback root_callback;
  
#ifdef __cplusplus
  class declaration *decl;
#endif
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

#endif /* TEST_GTY_INPUT_H */
