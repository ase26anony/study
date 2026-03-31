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
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  struct tree *tree_ptr;           /* TYPE_POINTER */
  union node_ptr *node;            /* TYPE_POINTER to TYPE_UNION */
  int data;                        /* TYPE_SCALAR */
  gty_string name;                 /* TYPE_STRING */
  gty_callback callback;           /* TYPE_CALLBACK */
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree {
  struct list * GTY((length ("%h.list_count"))) children[10];  /* TYPE_ARRAY of TYPE_POINTER */
  int list_count;                                              /* TYPE_SCALAR */
  tree_code code;                                             /* TYPE_SCALAR */
  struct tree * GTY((tag ("0"))) left;                       /* TYPE_POINTER */
  struct tree * GTY((tag ("1"))) right;                      /* TYPE_POINTER */
  
  /* Variable length array */
  struct list ** GTY((variable_length)) more_children;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct GTY(()) tree tree_t;

/* Additional structure for more coverage */
struct GTY(()) complex_struct {
  tree_t *root;                            /* TYPE_POINTER to TYPE_USER_STRUCT */
  union node_ptr nodes[5];                 /* TYPE_ARRAY of TYPE_UNION */
  struct {
    struct list *head;                     /* TYPE_POINTER in nested structure */
    struct list *tail;                     /* TYPE_POINTER */
  } GTY(()) list_info;
  
  /* Nested structure definition */
  struct GTY(()) inner_struct {
    struct tree *inner_tree;               /* TYPE_POINTER */
    int inner_value;                       /* TYPE_SCALAR */
  } inner;
};

/* TYPE_UNDEFINED: Forward declared structure that's never defined */
struct GTY(()) undefined_struct;

/* Structure referencing undefined type */
struct GTY(()) has_undefined {
  struct undefined_struct *undef_ptr;      /* TYPE_POINTER to TYPE_UNDEFINED */
  struct list *defined_ptr;                /* TYPE_POINTER for contrast */
};

#endif /* TEST_GTY_INPUT_H */
