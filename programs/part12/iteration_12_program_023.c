/* Test header for gengtype coverage of type state writing */
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
typedef void (* GTY((callback)) gty_callback_func)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string description;          /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  tree_code code;                  /* TYPE_SCALAR */
  struct list * GTY((tag ("0"))) children;  /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct tree * GTY((length ("4"))) fixed_array[4];
  
  /* TYPE_ARRAY: Variable-length array */
  struct list ** GTY((length ("%h.child_count"))) var_array;
  int child_count;
  
  gty_callback_func callback;      /* TYPE_CALLBACK */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_ptr {
  struct tree * GTY((tag ("1"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("2"))) ptr_list;    /* TYPE_POINTER */
  int type_tag;                               /* TYPE_SCALAR */
};

/* TYPE_STRUCT with nested union */
struct GTY(()) container {
  union node_ptr content;          /* TYPE_UNION */
  struct container * GTY((chain_next ("%h.next"))) next;
};

/* TYPE_ARRAY: Array type definition */
typedef tree_t * GTY((length ("%h.count"))) tree_array[];

/* TYPE_POINTER: Pointer type definition */
typedef struct list * GTY(()) list_ptr;

/* Complex structure using all features */
struct GTY(()) complex_struct {
  /* Chain pointers */
  struct complex_struct * GTY((chain_next ("%h.next"))) next;
  struct complex_struct * GTY((chain_prev ("%h.prev"))) prev;
  
  /* Various pointer types */
  tree_t *primary;                 /* TYPE_POINTER */
  list_ptr secondary;              /* TYPE_POINTER (via typedef) */
  
  /* Union field */
  union node_ptr optional;         /* TYPE_UNION */
  
  /* Arrays */
  tree_array dynamic_array;        /* TYPE_ARRAY */
  int count;
  
  /* String */
  gty_string name;                 /* TYPE_STRING */
  
  /* Scalar types */
  int id;                          /* TYPE_SCALAR */
  unsigned long flags;             /* TYPE_SCALAR */
  
  /* Callback */
  gty_callback_func notify;        /* TYPE_CALLBACK */
  
  /* Skip some fields */
  void * GTY((skip)) opaque;
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
private:
  tree_t *decl_tree;               /* TYPE_POINTER */
  complex_struct *context;         /* TYPE_POINTER */
  
public:
  declaration() : decl_tree(0), context(0) {}
  virtual ~declaration() {}
  
  void set_tree(tree_t *t) { decl_tree = t; }
  tree_t *get_tree() { return decl_tree; }
};
#endif

/* Root structure to tie everything together */
struct GTY(()) root_container {
  complex_struct *complex;         /* TYPE_POINTER */
  tree_t *tree_root;               /* TYPE_POINTER */
  struct list *list_head;          /* TYPE_POINTER */
  
  #ifdef __cplusplus
  declaration *decl;               /* TYPE_POINTER to TYPE_LANG_STRUCT */
  #endif
  
  /* Array of unions */
  union node_ptr GTY((length ("10"))) union_array[10];
  
  /* Two-dimensional array */
  tree_t * GTY((length ("%h.dim1"))) matrix[5][3];
  int dim1;
};

#endif /* TEST_GTY_INPUT_H */
