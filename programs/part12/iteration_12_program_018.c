/* Test header for gengtype coverage of TYPE_* classifications */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  CODE_ERROR,
  CODE_IDENTIFIER,
  CODE_TYPE,
  CODE_DECL
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic structure */
struct GTY(()) base_struct {
  int scalar_field;           /* TYPE_SCALAR */
  gty_string name;            /* TYPE_STRING */
  gty_callback callback_fn;   /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct * GTY((skip)) ptr;  /* TYPE_POINTER */
  int count;
} user_struct_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  struct base_struct * GTY((length ("10"))) fixed_array[10];
  
  /* Variable-length array */
  struct user_struct * GTY((variable_length)) var_array;
  int var_length;
  
  /* Nested array */
  struct base_struct * GTY((length ("%h.dim1 * %h.dim2"))) nested_array[5][3];
  int dim1, dim2;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.tag"))) tagged_union {
  int tag;  /* discriminator */
  
  struct base_struct * GTY((tag ("0"))) as_base;
  struct array_container * GTY((tag ("1"))) as_array;
  user_struct_t * GTY((tag ("2"))) as_user;
};

/* TYPE_POINTER: Pointer-only structure */
struct GTY(()) pointer_network {
  struct pointer_network * GTY((chain_next ("%h.next"))) next;
  struct pointer_network * GTY((chain_prev ("%h.prev"))) prev;
  struct base_struct *direct;
  struct array_container *indirect;
  union tagged_union *variant;
};

/* Linked list using chain_next */
struct GTY((chain_next ("%h.next"))) linked_item {
  struct linked_item *next;
  struct base_struct *data;
  int id;
};

/* Nested structure for complex dependencies */
struct GTY(()) complex_node {
  union tagged_union choice;
  struct array_container arrays;
  struct pointer_network *network;
  struct linked_item * GTY((length ("%h.list_size"))) item_list;
  int list_size;
  gty_string description;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;

/* Mutual recursion structure */
struct GTY(()) tree_list {
  struct tree_node * GTY((tag ("0"))) as_tree;
  struct tree_list * GTY((tag ("1"))) as_list;
  int tag;
};

struct GTY(()) tree_node {
  tree_code code;                    /* TYPE_SCALAR */
  gty_string name;                   /* TYPE_STRING */
  struct tree_node *left;            /* TYPE_POINTER */
  struct tree_node *right;           /* TYPE_POINTER */
  union {
    struct tree_list *children;      /* TYPE_UNION within */
    int value;
  } GTY((desc ("%0.has_children"))) u;
  int has_children;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY(()) lang_class {
public:
  struct tree_node *root;
  struct complex_node *data;
  gty_string class_name;
  
  /* Method to ensure class is processed */
  virtual void dummy() {}
};

#ifdef __cplusplus
}
#endif

/* Root structure containing all types */
struct GTY(()) root_container {
  struct base_struct base;           /* TYPE_STRUCT */
  user_struct_t user;                /* TYPE_USER_STRUCT */
  union tagged_union union_field;    /* TYPE_UNION */
  struct pointer_network *network;   /* TYPE_POINTER */
  struct array_container arrays;     /* TYPE_ARRAY */
  class lang_class *lang_obj;        /* TYPE_LANG_STRUCT */
  int scalar_data;                   /* TYPE_SCALAR */
  gty_string info;                   /* TYPE_STRING */
  gty_callback handler;              /* TYPE_CALLBACK */
  struct tree_node *tree;            /* Additional TYPE_POINTER */
};

/* Global variable to ensure types are referenced */
extern struct root_container * GTY((root)) global_root;

#endif /* TEST_GTY_INPUT_H */
