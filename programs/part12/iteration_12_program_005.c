/* Test header for gengtype coverage of type state writing */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;

/* TYPE_SCALAR: Enumeration type */
enum tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
};

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* Skip to avoid infinite recursion */
  int data;
  gty_string_t name;  /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;  /* TYPE_SCALAR */
  struct list *children;  /* TYPE_POINTER */
  gty_callback_t callback;  /* TYPE_CALLBACK */
  int value;
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;   /* TYPE_POINTER */
  int type;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) graph {
  struct tree * GTY((length ("%h.node_count"))) nodes[10];  /* Fixed array */
  struct list ** GTY((variable_length)) edges;  /* Variable length array */
  int node_count;
  int edge_count;
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct graph * GTY(()) graph_ptr_t;

/* Nested structure to ensure deep traversal */
struct GTY(()) container {
  union node_ptr item;      /* TYPE_UNION */
  struct graph *graph;      /* TYPE_POINTER */
  tree_t *tree_array[5];    /* TYPE_ARRAY */
  gty_string_t strings[3];  /* TYPE_ARRAY of TYPE_STRING */
};

/* For TYPE_LANG_STRUCT - must be in C++ context */
#ifdef __cplusplus
class GTY((user)) declaration {
  tree_t *decl_tree;        /* TYPE_POINTER */
  struct container *cont;   /* TYPE_POINTER */
  gty_string_t decl_name;   /* TYPE_STRING */
  
public:
  declaration() : decl_tree(0), cont(0), decl_name(0) {}
  virtual ~declaration() {}
};
#endif

/* Root structure that ties everything together */
struct GTY(()) root_struct {
  struct container main_container;  /* TYPE_STRUCT */
  graph_ptr_t graph;                /* TYPE_POINTER (via typedef) */
  struct list *list_chain;          /* TYPE_POINTER */
  #ifdef __cplusplus
  class declaration *decl;          /* TYPE_LANG_STRUCT */
  #endif
  gty_callback_t callbacks[2];      /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

#endif /* TEST_GTY_INPUT_H */
