/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

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
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  struct tree *tree_ptr;           /* TYPE_POINTER */
  union node_ptr *node;            /* TYPE_POINTER to union */
  int scalar_data;                 /* TYPE_SCALAR */
  gty_string_t name;               /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct list GTY((user)) user_list_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_ptr {
  struct tree * GTY((tag ("0"))) as_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) as_list;  /* TYPE_POINTER */
  int type_tag;                            /* TYPE_SCALAR */
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree {
  tree_code code;                          /* TYPE_SCALAR */
  
  /* Fixed-size array of pointers */
  struct list * GTY((length ("5"))) fixed_array[5];  /* TYPE_ARRAY */
  
  /* Variable-length array */
  struct tree ** GTY((variable_length)) children;    /* TYPE_ARRAY */
  unsigned int num_children;                         /* TYPE_SCALAR */
  
  /* Array within union */
  union {
    struct list * GTY((length ("3"))) small_list[3]; /* TYPE_ARRAY */
    struct tree *large_tree;                         /* TYPE_POINTER */
  } GTY((desc ("%h.code == TREE_CODE_A"))) u;
  
  gty_callback_t callback;                  /* TYPE_CALLBACK */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree * GTY((user)) tree_ptr_t;

/* Complex nested structure for additional coverage */
struct GTY(()) complex_struct {
  /* Nested structure */
  struct GTY(()) inner {
    struct tree *ptr;                      /* TYPE_POINTER */
    int value;                             /* TYPE_SCALAR */
  } inner_obj;
  
  /* Pointer to array */
  struct list ** GTY((length ("%h.count"))) ptr_array; /* TYPE_ARRAY */
  int count;                               /* TYPE_SCALAR */
  
  /* Union with array */
  union {
    tree_ptr_t single;                     /* TYPE_POINTER */
    struct tree * GTY((length ("2"))) pair[2]; /* TYPE_ARRAY */
  } choice;
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
  struct tree *decl_tree;                  /* TYPE_POINTER */
  user_list_t *decl_list;                  /* TYPE_POINTER */
  gty_string_t decl_name;                  /* TYPE_STRING */
  
public:
  declaration() : decl_tree(0), decl_list(0), decl_name(0) {}
  virtual ~declaration() {}
  
  void set_tree(struct tree *t) { decl_tree = t; }
  struct tree *get_tree() { return decl_tree; }
};
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct list *main_list;                  /* TYPE_POINTER */
  struct tree *main_tree;                  /* TYPE_POINTER */
  union node_ptr main_node;                /* TYPE_UNION */
  struct complex_struct *complex;          /* TYPE_POINTER */
#ifdef __cplusplus
  class declaration *decl;                 /* TYPE_POINTER to TYPE_LANG_STRUCT */
#endif
  gty_callback_t callbacks[3];             /* TYPE_ARRAY of TYPE_CALLBACK */
};

#endif /* TEST_GTY_INPUT_H */
