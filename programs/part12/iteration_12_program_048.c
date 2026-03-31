/* Test file to cover all gengtype-state.cc type classifications */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations for type dependencies */
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
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string description;          /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  tree_code code;                  /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree ** GTY((length ("%h.child_count"))) var_array;
  int child_count;
  
  gty_callback callback;           /* TYPE_CALLBACK */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_ptr {
  struct tree * GTY((tag ("0"))) as_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) as_list;    /* TYPE_POINTER */
  int type_tag;                              /* TYPE_SCALAR */
};

/* More complex TYPE_STRUCT with nested union */
struct GTY(()) complex_struct {
  union node_ptr primary;          /* TYPE_UNION */
  union node_ptr secondary;        /* TYPE_UNION */
  
  /* Nested structure */
  struct GTY(()) {
    struct tree *left;             /* TYPE_POINTER */
    struct tree *right;            /* TYPE_POINTER */
  } nested;
  
  /* Array of unions */
  union node_ptr GTY((length ("5"))) union_array[5];
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  struct tree *decl_tree;          /* TYPE_POINTER */
  struct list *decl_list;          /* TYPE_POINTER */
  gty_string name;                 /* TYPE_STRING */
  
  /* TYPE_ARRAY: Array within class */
  struct complex_struct * GTY((length ("%h.count"))) decl_array;
  int count;
  
  virtual ~declaration() {}
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct tree *root_tree;          /* TYPE_POINTER */
  struct list *root_list;          /* TYPE_POINTER */
  union node_ptr root_union;       /* TYPE_UNION */
  struct complex_struct complex;   /* TYPE_STRUCT */
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
  
  /* Multiple pointer types in array */
  void * GTY((atomic)) opaque_ptrs[3];
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

/* Function to create type dependencies */
void GTY((user)) init_types(struct root_container *root);

#endif /* TEST_GTY_INPUT_H */
