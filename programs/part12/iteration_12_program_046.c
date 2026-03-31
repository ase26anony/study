/* Test header to trigger all gengtype type classifications */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
enum tree_code;
typedef void (*callback_func)(void *);

/* TYPE_SCALAR: Enumeration type */
enum tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
};

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Callback function pointer */
typedef void (* GTY((callback)) gty_callback)(struct tree *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string description;          /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;             /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  gty_callback callback;           /* TYPE_CALLBACK */
  unsigned int length;             /* TYPE_SCALAR */
} tree_t;

/* TYPE_ARRAY: Structure with array field */
struct GTY(()) tree_container {
  tree_t * GTY((length ("%h.length"))) array_field[10];  /* TYPE_ARRAY */
  int length;                                            /* TYPE_SCALAR */
  tree_t * GTY((variable_length)) var_array;            /* TYPE_ARRAY (variable) */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef tree_t * GTY((atomic)) tree_ptr;

/* Complex structure using all types */
struct GTY(()) complex_struct {
  /* TYPE_STRUCT nested field */
  struct list header;              /* TYPE_STRUCT */
  
  /* TYPE_UNION field */
  union node_ptr node;             /* TYPE_UNION */
  
  /* TYPE_POINTER fields */
  tree_ptr root;                   /* TYPE_POINTER */
  struct tree_container *container; /* TYPE_POINTER */
  
  /* TYPE_ARRAY field */
  gty_string GTY((length ("5"))) tags[5];  /* TYPE_ARRAY of TYPE_STRING */
  
  /* TYPE_SCALAR fields */
  int id;                          /* TYPE_SCALAR */
  enum tree_code default_code;     /* TYPE_SCALAR */
  
  /* TYPE_CALLBACK field */
  gty_callback notify;             /* TYPE_CALLBACK */
  
  /* TYPE_STRING field */
  gty_string name;                 /* TYPE_STRING */
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct undefined_struct;

/* Structure referencing undefined type */
struct GTY(()) has_undefined {
  struct undefined_struct *undef;  /* Will be TYPE_UNDEFINED */
  struct complex_struct *defined;  /* TYPE_POINTER */
};

#endif /* TEST_GTY_INPUT_H */
