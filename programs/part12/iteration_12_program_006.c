/* test-gty-input.h - Comprehensive GTY type definitions for coverage testing */

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
  struct list * GTY((tag ("0"))) children;  /* TYPE_POINTER */
  gty_string_t str;                /* TYPE_STRING */
  int value;                       /* TYPE_SCALAR */
} tree_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  tree_t * GTY((length ("%h.count"))) array_field[10];  /* TYPE_ARRAY of TYPE_POINTER */
  int count;                        /* TYPE_SCALAR */
  
  /* Variable length array */
  struct list * GTY((variable_length)) var_array[1];  /* TYPE_ARRAY */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* More complex TYPE_STRUCT with nested types */
struct GTY(()) complex_struct {
  union node_ptr node;              /* TYPE_UNION */
  struct array_container *container; /* TYPE_POINTER */
  gty_callback_t callback;          /* TYPE_CALLBACK */
  
  /* Nested structure */
  struct GTY(()) nested {
    tree_t *tree_ptr;               /* TYPE_POINTER */
    int id;                         /* TYPE_SCALAR */
  } nested_data;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  tree_t *decl_tree;                /* TYPE_POINTER */
  gty_string_t decl_name;           /* TYPE_STRING */
  
  /* Array of pointers */
  struct list * GTY((length ("5"))) decl_list[5];  /* TYPE_ARRAY */
  
private:
  int decl_id;                      /* TYPE_SCALAR */
};

#ifdef __cplusplus
}
#endif

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

/* Root structure that ties everything together */
struct GTY(()) root_container {
  struct complex_struct *complex;   /* TYPE_POINTER */
  class declaration *decl;          /* TYPE_POINTER (to TYPE_LANG_STRUCT) */
  union node_ptr root_node;         /* TYPE_UNION */
  gty_callback_t root_callback;     /* TYPE_CALLBACK */
  
  /* Chain of lists */
  struct list *first_list;          /* TYPE_POINTER */
  
  /* Pointer to undefined type */
  struct undefined_struct *undef;   /* TYPE_POINTER (to TYPE_UNDEFINED) */
};

#endif /* TEST_GTY_INPUT_H */
