/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Enumeration type */
typedef enum {
  CODE_NONE,
  CODE_LIST,
  CODE_TREE
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Callback function pointer */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic GC-tracked structure */
struct GTY((chain_next ("%h.next"))) list_node {
  struct list_node *next;  /* TYPE_POINTER */
  int value;               /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: Typedef of a struct */
typedef struct list_node * GTY((user)) list_ptr;

/* TYPE_UNION: Discriminated union with pointers */
union GTY((desc ("%0.type"))) node_union {
  struct list_node * GTY((tag ("CODE_LIST"))) as_list;  /* TYPE_POINTER */
  struct tree_node * GTY((tag ("CODE_TREE"))) as_tree;  /* TYPE_POINTER */
  tree_code type;                                       /* TYPE_SCALAR */
};

/* TYPE_ARRAY: Structure with array of pointers */
struct GTY(()) tree_node {
  tree_code code;                               /* TYPE_SCALAR */
  gty_string name;                              /* TYPE_STRING */
  struct list_node * GTY((length ("%h.child_count"))) children[5]; /* TYPE_ARRAY */
  int child_count;
};

/* TYPE_LANG_STRUCT: C++ class (frontend structure) */
#ifdef __cplusplus
class GTY((user)) declaration {
public:
  struct tree_node *node;   /* TYPE_POINTER */
  gty_callback callback;    /* TYPE_CALLBACK */
};
#endif

/* Root container structure that ties everything together */
struct GTY(()) root_container {
  struct list_node *list_head;      /* TYPE_POINTER */
  union node_union current_node;    /* TYPE_UNION */
  struct tree_node *tree_root;      /* TYPE_POINTER */
#ifdef __cplusplus
  class declaration *decl;          /* TYPE_LANG_STRUCT */
#endif
  gty_string description;           /* TYPE_STRING */
};

#endif /* TEST_GTY_INPUT_H */
