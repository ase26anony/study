/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_VOID,
  TREE_INTEGER,
  TREE_REAL,
  TREE_STRING,
  TREE_COMPLEX,
  TREE_VECTOR
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_fn)(void *data);

/* Forward declarations */
struct tree_node;
struct list_node;
union node_union;
class declaration;

/* TYPE_STRUCT: Basic structure with pointer fields */
struct GTY(()) tree_node {
  tree_code code;              /* TYPE_SCALAR */
  gty_string_t name;           /* TYPE_STRING */
  struct tree_node * GTY((skip)) left;  /* TYPE_POINTER with skip */
  struct tree_node *right;     /* TYPE_POINTER */
  gty_callback_fn callback;    /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct tree_node my_tree GTY(());

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) list_node {
  int length;
  /* Fixed-size array of pointers */
  struct tree_node * GTY((length ("10"))) children[10];
  /* Variable-length array */
  struct tree_node ** GTY((variable_length)) more_children;
  /* Chain for linked list */
  struct list_node * GTY((chain_next ("%h.next"))) next;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_union {
  int type;                    /* discriminator */
  struct tree_node * GTY((tag ("0"))) as_tree;
  struct list_node * GTY((tag ("1"))) as_list;
  my_tree * GTY((tag ("2"))) as_my_tree;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((operator delete)) declaration {
public:
  struct tree_node *decl_tree;     /* TYPE_POINTER */
  union node_union decl_union;     /* TYPE_UNION */
  struct list_node **decl_list;    /* TYPE_POINTER to TYPE_POINTER */
  
  /* Array of strings */
  gty_string_t GTY((length ("%h.num_attrs"))) *attributes;
  int num_attrs;
  
  /* Nested structure within class */
  struct GTY(()) nested {
    struct tree_node *nested_tree;
    int value;
  } nested_data;
};

#ifdef __cplusplus
}
#endif

/* TYPE_POINTER: Typedef for pointer type */
typedef struct list_node * GTY(()) list_ptr;

/* Complex structure using all types */
struct GTY(()) root_struct {
  /* Basic types */
  my_tree *root_tree;              /* TYPE_POINTER to TYPE_USER_STRUCT */
  union node_union root_union;     /* TYPE_UNION */
  list_ptr root_list;              /* TYPE_POINTER (typedef) */
  
  /* Array of unions */
  union node_union GTY((length ("%h.union_count"))) union_array[5];
  int union_count;
  
  /* Pointer to callback */
  gty_callback_fn GTY((skip)) *callbacks;
  
  /* For TYPE_UNDEFINED coverage */
  struct undefined_type *undef_ptr; /* Forward declared, not defined */
  
  /* Nested structure */
  struct GTY(()) {
    int x;
    struct tree_node *nested_ptr;
  } anonymous_struct;
};

/* Global variable to ensure types are used */
extern struct root_struct * GTY(()) global_root;

#endif /* TEST_GTY_INPUT_H */
