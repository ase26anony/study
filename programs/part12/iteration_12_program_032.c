/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* TYPE_STRUCT: Basic structure with pointer fields */
struct GTY((chain_next ("%h.next"))) list_node {
  struct list_node * GTY((skip)) next;  /* TYPE_POINTER */
  int GTY((skip)) data;                 /* TYPE_SCALAR */
  gty_string_t GTY((skip)) name;        /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree_node {
  tree_code GTY((skip)) code;           /* TYPE_SCALAR */
  struct tree_node * GTY((skip)) left;  /* TYPE_POINTER */
  struct tree_node * GTY((skip)) right; /* TYPE_POINTER */
  gty_callback_t GTY((skip)) callback;  /* TYPE_CALLBACK */
} tree_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  int GTY((skip)) count;
  
  /* Fixed-size array of pointers */
  tree_t * GTY((skip)) fixed_array[10];  /* TYPE_ARRAY of TYPE_POINTER */
  
  /* Variable-length array */
  struct list_node * GTY((length ("%h.count"))) var_array[1]; /* TYPE_ARRAY */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_union {
  int type;                            /* discriminator - TYPE_SCALAR */
  struct list_node * GTY((skip)) list; /* TYPE_POINTER */
  tree_t * GTY((skip)) tree;           /* TYPE_POINTER */
  struct array_container * GTY((skip)) array; /* TYPE_POINTER */
};

/* TYPE_STRUCT with nested union */
struct GTY(()) complex_struct {
  struct list_node * GTY((skip)) head;      /* TYPE_POINTER */
  union node_union GTY((skip)) data;        /* TYPE_UNION */
  struct array_container GTY((skip)) arrays; /* TYPE_STRUCT */
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;

/* Another structure that references the forward declaration */
struct GTY(()) recursive_struct {
  int value;
  struct forward_decl_struct * GTY((skip)) forward_ptr; /* TYPE_POINTER */
  struct recursive_struct * GTY((skip)) self_ptr;       /* TYPE_POINTER */
};

/* Complete the forward declaration */
struct GTY(()) forward_decl_struct {
  char * GTY((skip)) name;
  struct recursive_struct * GTY((skip)) recursive; /* TYPE_POINTER */
};

/* Root structure that ties everything together */
struct GTY(()) root_container {
  struct complex_struct GTY((skip)) complex;      /* TYPE_STRUCT */
  struct recursive_struct * GTY((skip)) recursive; /* TYPE_POINTER */
  struct forward_decl_struct GTY((skip)) forward;  /* TYPE_STRUCT */
  union node_union GTY((skip)) union_field;        /* TYPE_UNION */
};

#endif /* TEST_GTY_INPUT_H */
