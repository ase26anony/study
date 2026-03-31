/* Test header for gengtype coverage of TYPE_* classifications */

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
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string_t name;               /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  struct tree *left;               /* TYPE_POINTER */
  struct tree *right;              /* TYPE_POINTER */
  enum tree_code code;             /* TYPE_SCALAR */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("4"))) children[4];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree ** GTY((variable_length)) more_children;
  
  /* Callback field */
  gty_callback_t callback;         /* TYPE_CALLBACK */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct list * GTY(()) list_ptr_t;

/* Container structure using all types */
struct GTY(()) container {
  /* TYPE_STRUCT */
  struct tree *root;               /* TYPE_POINTER */
  
  /* TYPE_UNION */
  union node_ptr current;          /* TYPE_UNION */
  
  /* TYPE_ARRAY: Array of structures */
  struct list GTY((length ("%h.count"))) items[10];
  
  /* TYPE_POINTER: Pointer typedef */
  list_ptr_t list_head;            /* TYPE_POINTER */
  
  /* TYPE_SCALAR */
  int count;                       /* TYPE_SCALAR */
  
  /* TYPE_STRING */
  gty_string_t description;        /* TYPE_STRING */
  
  /* TYPE_ARRAY: Array of strings */
  gty_string_t GTY((length ("%h.tag_count"))) tags[5];
  
  int tag_count;                   /* TYPE_SCALAR */
};

/* Nested structure for more coverage */
struct GTY(()) nested {
  struct container * GTY((skip)) parent;      /* TYPE_POINTER */
  struct nested *sibling;                     /* TYPE_POINTER */
  struct nested *children[3];                 /* TYPE_ARRAY of TYPE_POINTER */
};

#endif /* TEST_GTY_INPUT_H */
