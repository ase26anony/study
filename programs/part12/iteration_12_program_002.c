/* Test header for gengtype coverage of type state writing */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;
typedef void (*callback_func)(void *);

/* TYPE_SCALAR: Enumeration type */
enum tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
};

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Callback function type */
typedef void (* GTY((callback)) my_callback_type)(struct tree *);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  struct tree *data;               /* TYPE_POINTER */
  int count;                       /* TYPE_SCALAR */
  gty_string name;                 /* TYPE_STRING */
  my_callback_type callback;       /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct list GTY((user)) user_list;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree {
  enum tree_code code;                      /* TYPE_SCALAR */
  
  /* Fixed-size array of pointers */
  struct list * GTY((length ("5"))) children[5];  /* TYPE_ARRAY of TYPE_POINTER */
  
  /* Variable-length array */
  struct tree ** GTY((length ("%h.var_len"))) var_children;  /* TYPE_ARRAY */
  int var_len;                               /* TYPE_SCALAR */
  
  /* Union member */
  union node_ptr GTY((skip)) alt_ptr;       /* TYPE_UNION */
  
  /* For chain linking */
  struct tree *parent;                      /* TYPE_POINTER */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree * GTY((user)) tree_ptr;

/* Nested structure for complex testing */
struct GTY(()) complex_struct {
  /* Array of unions */
  union node_ptr GTY((length ("3"))) nodes[3];  /* TYPE_ARRAY of TYPE_UNION */
  
  /* Pointer to array */
  struct list ** GTY((length ("%h.list_count"))) list_array;  /* TYPE_POINTER to TYPE_ARRAY */
  int list_count;                            /* TYPE_SCALAR */
  
  /* Multiple callback types */
  my_callback_type callbacks[2];             /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* For TYPE_LANG_STRUCT - C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
  struct tree *decl_tree;                    /* TYPE_POINTER */
  struct list *decl_list;                    /* TYPE_POINTER */
  gty_string decl_name;                      /* TYPE_STRING */
  
public:
  declaration() : decl_tree(0), decl_list(0), decl_name(0) {}
  virtual ~declaration() {}
  
  void set_tree(struct tree *t) { decl_tree = t; }
  struct tree *get_tree() { return decl_tree; }
};
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct tree *main_tree;                    /* TYPE_POINTER */
  struct list *main_list;                    /* TYPE_POINTER */
  union node_ptr main_union;                 /* TYPE_UNION */
  struct complex_struct *complex;            /* TYPE_POINTER */
  
  #ifdef __cplusplus
  class declaration *decl;                   /* TYPE_POINTER to TYPE_LANG_STRUCT */
  #endif
  
  /* Array of various types */
  tree_ptr tree_array[10];                   /* TYPE_ARRAY of TYPE_POINTER */
  my_callback_type callback_array[5];        /* TYPE_ARRAY of TYPE_CALLBACK */
  
  /* String array */
  gty_string GTY((length ("%h.str_count"))) strings;  /* TYPE_ARRAY of TYPE_STRING */
  int str_count;                             /* TYPE_SCALAR */
};

#endif /* TEST_GTY_INPUT_H */
