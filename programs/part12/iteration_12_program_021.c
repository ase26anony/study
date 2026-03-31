/* Comprehensive GTY type definitions to cover all switch cases in gengtype-state.cc */

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
  enum tree_code code;             /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  gty_string_t GTY((tag ("0"))) str; /* TYPE_STRING with tag */
  int value;                       /* TYPE_SCALAR */
} tree_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree_container {
  tree_t * GTY((length ("%h.count"))) children[10]; /* TYPE_ARRAY */
  int count;                        /* TYPE_SCALAR */
  tree_t * GTY((variable_length)) more_children; /* TYPE_ARRAY (variable) */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0.type == 1"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("0.type == 2"))) ptr_list;  /* TYPE_POINTER */
  int type;                          /* TYPE_SCALAR (discriminator) */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef tree_t * GTY((atomic)) tree_ptr;

/* Complex structure using all types */
struct GTY(()) complex_struct {
  /* TYPE_STRUCT nested field */
  struct inner {
    tree_ptr ptr;                  /* TYPE_POINTER */
    int id;                        /* TYPE_SCALAR */
  } GTY((skip)) inner_data;
  
  union node_ptr node;             /* TYPE_UNION */
  struct tree_container container; /* TYPE_STRUCT containing TYPE_ARRAY */
  gty_callback_t callback;         /* TYPE_CALLBACK */
  
  /* Array of pointers */
  struct list * GTY((length ("5"))) list_array[5]; /* TYPE_ARRAY */
  
  /* Nested structure array */
  struct {
    tree_t *item;                  /* TYPE_POINTER */
    gty_string_t name;             /* TYPE_STRING */
  } GTY((skip)) items[3];          /* TYPE_ARRAY of structures */
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  tree_t *decl_tree;               /* TYPE_POINTER */
  complex_struct *complex;         /* TYPE_POINTER */
  gty_string_t decl_name;          /* TYPE_STRING */
  
  /* Virtual method to ensure C++ class properties */
  virtual ~declaration() {}
  
private:
  int decl_id;                     /* TYPE_SCALAR */
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct list *first_list;         /* TYPE_POINTER */
  tree_t *root_tree;               /* TYPE_POINTER */
  union node_ptr current_node;     /* TYPE_UNION */
  struct complex_struct complex;   /* TYPE_STRUCT */
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
  gty_callback_t callbacks[2];     /* TYPE_ARRAY of TYPE_CALLBACK */
};

#endif /* TEST_GTY_INPUT_H */
