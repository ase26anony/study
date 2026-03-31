/* Test header to trigger all gengtype type classifications */
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

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string_t name;               /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;             /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  gty_callback_t callback;         /* TYPE_CALLBACK */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree * GTY((variable_length)) var_array[1];
  
  /* For variable_length arrays, we need a length field */
  unsigned int var_length;
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;   /* TYPE_POINTER */
  int type;                                  /* TYPE_SCALAR */
};

/* TYPE_STRUCT with nested union */
struct GTY(()) container {
  union node_ptr node;             /* TYPE_UNION */
  struct tree *root;               /* TYPE_POINTER */
  
  /* TYPE_ARRAY of unions */
  union node_ptr GTY((length ("5"))) node_array[5];
};

/* TYPE_POINTER: Typedef for a pointer type */
typedef struct container * GTY(()) container_ptr_t;

/* Complex structure with multiple pointer types */
struct GTY(()) complex_struct {
  container_ptr_t container;       /* TYPE_POINTER (via typedef) */
  struct list * GTY((chain_next ("%h.next_link"))) next_link;
  struct complex_struct *prev;     /* TYPE_POINTER */
  
  /* Nested structure */
  struct GTY(()) {
    struct tree *inner_tree;       /* TYPE_POINTER */
    int inner_data;                /* TYPE_SCALAR */
  } nested;
};

/* For TYPE_LANG_STRUCT - C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
  tree_t *decl_tree;               /* TYPE_POINTER */
  struct list *decl_list;          /* TYPE_POINTER */
  
public:
  declaration() : decl_tree(0), decl_list(0) {}
  virtual ~declaration() {}
  
  void set_tree(tree_t *t) { decl_tree = t; }
  tree_t *get_tree() { return decl_tree; }
};
#endif

/* Root structure that references everything */
struct GTY(()) root_struct {
  struct complex_struct *complex;  /* TYPE_POINTER */
  union node_ptr main_node;        /* TYPE_UNION */
  tree_t main_tree;                /* TYPE_STRUCT */
  gty_string_t description;        /* TYPE_STRING */
  
  #ifdef __cplusplus
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
  #endif
};

#endif /* TEST_GTY_INPUT_H */
