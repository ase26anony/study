/* Test header for gengtype coverage of all type classifications */

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
  gty_callback_t callback;         /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;             /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree * GTY((variable_length)) var_array[1];
  
  int var_length;  /* For variable_length array */
} tree_t;  /* TYPE_USER_STRUCT via typedef */

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct list * GTY(()) list_ptr_t;

/* Complex structure using all types */
struct GTY(()) container {
  /* TYPE_STRUCT nested field */
  struct list head;                 /* TYPE_STRUCT */
  
  /* TYPE_POINTER fields */
  tree_t *root;                     /* TYPE_POINTER */
  list_ptr_t current;               /* TYPE_POINTER */
  
  /* TYPE_UNION field */
  union node_ptr active;            /* TYPE_UNION */
  
  /* TYPE_ARRAY of unions */
  union node_ptr GTY((length ("5"))) union_array[5];
  
  /* TYPE_ARRAY of pointers */
  tree_t * GTY((length ("%h.count"))) dynamic_array[1];
  int count;                        /* For dynamic array length */
  
  /* TYPE_SCALAR fields */
  int id;
  enum tree_code last_code;
  
  /* TYPE_STRING field */
  gty_string_t description;
  
  /* TYPE_CALLBACK field */
  gty_callback_t handler;
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  tree_t *decl_tree;                /* TYPE_POINTER */
  container *parent;                /* TYPE_POINTER */
  gty_string_t ident;               /* TYPE_STRING */
  
private:
  int line;                         /* TYPE_SCALAR */
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  container main;                   /* TYPE_STRUCT */
  declaration *decl;                /* TYPE_POINTER */
  struct undefined_struct *undef;   /* TYPE_POINTER to undefined */
};

#endif /* TEST_GTY_INPUT_H */
