/* Test file to cover all type classifications in gengtype-state.cc */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string description;          /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  tree_code code;                  /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  gty_callback callback;           /* TYPE_CALLBACK */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree ** GTY((length ("%h.var_len"))) var_array;
  int var_len;                     /* TYPE_SCALAR */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int type_tag;                               /* TYPE_SCALAR */
};

/* More complex TYPE_STRUCT with nested union */
struct GTY(()) complex_struct {
  union node_ptr node;             /* TYPE_UNION */
  struct complex_struct *parent;   /* TYPE_POINTER */
  
  /* Nested structure */
  struct GTY(()) {
    int x;                         /* TYPE_SCALAR */
    struct list *items;            /* TYPE_POINTER */
  } nested;
};

/* TYPE_ARRAY: Standalone array type */
typedef struct list * GTY((length ("5"))) list_array[5];

/* For TYPE_LANG_STRUCT - C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
  tree_t *decl_tree;               /* TYPE_POINTER */
  gty_string name;                 /* TYPE_STRING */
  
public:
  declaration() : decl_tree(0), name(0) {}
  virtual ~declaration() {}
  
  void set_tree(tree_t *t) { decl_tree = t; }
};
#endif

/* Root container structure that references everything */
struct GTY(()) root_container {
  struct complex_struct *complex;  /* TYPE_POINTER */
  list_array array_field;          /* TYPE_ARRAY */
  union node_ptr current;          /* TYPE_UNION */
  
#ifdef __cplusplus
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
#endif
  
  /* Atomic pointer (won't be traced) */
  void * GTY((atomic)) opaque_ptr;
};

#endif /* TEST_GTY_INPUT_H */
