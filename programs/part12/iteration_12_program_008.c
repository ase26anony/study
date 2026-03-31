/* Test file to cover all gengtype-state.cc type classifications */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations for type dependencies */
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
  struct tree *left;               /* TYPE_POINTER */
  struct tree *right;              /* TYPE_POINTER */
  tree_code code;                  /* TYPE_SCALAR */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("5"))) children[5];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree ** GTY((variable_length)) more_children;
  
  gty_callback callback_fn;        /* TYPE_CALLBACK */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* More complex TYPE_STRUCT with nested union */
struct GTY(()) complex_struct {
  union node_ptr node;             /* TYPE_UNION */
  
  /* Chain of pointers with skip */
  struct complex_struct * GTY((skip("&%h.skip_field"))) chain_next;
  struct complex_struct *skip_field;
  
  /* Nested structure */
  struct GTY(()) {
    tree_t *inner_tree;            /* TYPE_POINTER */
    int inner_data;                /* TYPE_SCALAR */
  } nested;
};

/* TYPE_ARRAY: Standalone array type */
typedef struct list * GTY((length ("%h.count"))) list_array;

/* Container structure using the array type */
struct GTY(()) container {
  int count;                       /* TYPE_SCALAR */
  list_array items;                /* TYPE_ARRAY */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef tree_t * GTY(()) tree_ptr;

/* Another structure using the pointer typedef */
struct GTY(()) wrapper {
  tree_ptr wrapped;                /* TYPE_POINTER via typedef */
  struct container *cont;          /* TYPE_POINTER */
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
  tree_t *decl_tree;               /* TYPE_POINTER */
  struct list *decl_list;          /* TYPE_POINTER */
  int decl_id;                     /* TYPE_SCALAR */
  
public:
  declaration() : decl_tree(0), decl_list(0), decl_id(0) {}
  virtual ~declaration() {}
  
  void set_tree(tree_t *t) { decl_tree = t; }
};
#endif

/* Root structure that ties everything together */
struct GTY(()) root_type {
  struct complex_struct *complex;  /* TYPE_POINTER */
  struct wrapper *wrap;            /* TYPE_POINTER */
  struct container containers[3];  /* TYPE_ARRAY of TYPE_STRUCT */
  
#ifdef __cplusplus
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
#endif
  
  /* Union with anonymous struct */
  union GTY(()) {
    struct {
      tree_t *anon_tree;           /* TYPE_POINTER */
      int anon_flag;               /* TYPE_SCALAR */
    };
    long long anon_data;           /* TYPE_SCALAR */
  } anonymous;
};

#endif /* TEST_GTY_INPUT_H */
