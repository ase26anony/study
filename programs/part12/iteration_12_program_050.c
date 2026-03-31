/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

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
  gty_string_t GTY((tag ("0"))) str; /* TYPE_STRING */
  gty_callback_t callback;         /* TYPE_CALLBACK */
} tree_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  int length;                      /* TYPE_SCALAR */
  
  /* Fixed-size array of pointers */
  struct tree * GTY((length ("10"))) fixed_array[10]; /* TYPE_ARRAY of TYPE_POINTER */
  
  /* Variable-length array */
  struct list * GTY((length ("%h.length"))) var_array[1]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  int type;                        /* TYPE_SCALAR - discriminant */
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  struct array_container * GTY((tag ("2"))) ptr_array; /* TYPE_POINTER */
};

/* TYPE_STRUCT with nested union */
struct GTY(()) complex_struct {
  union node_ptr node;             /* TYPE_UNION */
  struct complex_struct * GTY((chain_next ("%h.next"))) next; /* TYPE_POINTER */
  
  /* Nested structure */
  struct GTY(()) nested {
    struct tree *tree_ptr;         /* TYPE_POINTER */
    int value;                     /* TYPE_SCALAR */
  } nested_obj;
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY((operator delete)) declaration {
  struct tree *decl_tree;          /* TYPE_POINTER */
  struct list *decl_list;          /* TYPE_POINTER */
  gty_string_t decl_name;          /* TYPE_STRING */
  
public:
  declaration() : decl_tree(0), decl_list(0), decl_name(0) {}
  virtual ~declaration() {}
  
  void set_tree(struct tree *t) { decl_tree = t; }
  struct tree *get_tree() { return decl_tree; }
};
#endif /* __cplusplus */

/* TYPE_POINTER in typedef */
typedef struct complex_struct * GTY((skip)) complex_ptr_t;

/* Root structure that references everything */
struct GTY(()) root_container {
  struct tree *root_tree;          /* TYPE_POINTER */
  struct list *root_list;          /* TYPE_POINTER */
  union node_ptr root_node;        /* TYPE_UNION */
  struct array_container root_array; /* TYPE_STRUCT */
  complex_ptr_t complex;           /* TYPE_POINTER */
  
  #ifdef __cplusplus
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
  #endif
  
  /* Array of callbacks */
  gty_callback_t GTY((length ("3"))) callbacks[3]; /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

#endif /* TEST_GTY_INPUT_H */
