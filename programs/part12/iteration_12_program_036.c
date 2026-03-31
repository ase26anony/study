/* Test header to cover all gengtype-state.cc type classifications */

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
  struct tree * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct list * GTY((variable_length)) var_array[1];
  
  int length;  /* For variable_length array */
  
  /* Callback field */
  gty_callback callback;           /* TYPE_CALLBACK */
} tree_user;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* More complex TYPE_STRUCT with nested union */
struct GTY(()) complex_struct {
  union node_ptr node;             /* TYPE_UNION */
  
  /* Nested structure */
  struct GTY(()) {
    struct tree *left;             /* TYPE_POINTER */
    struct tree *right;            /* TYPE_POINTER */
  } nested;
  
  /* Array of unions */
  union node_ptr GTY((length ("5"))) union_array[5];  /* TYPE_ARRAY of TYPE_UNION */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct complex_struct * GTY(()) complex_ptr;

/* Container structure using all types */
struct GTY(()) container {
  /* Various pointer types */
  struct tree *tree_ptr;           /* TYPE_POINTER */
  struct list *list_ptr;           /* TYPE_POINTER */
  complex_ptr cptr;                /* TYPE_POINTER (typedef) */
  
  /* Union field */
  union node_ptr current;          /* TYPE_UNION */
  
  /* String array */
  gty_string GTY((length ("%h.str_count"))) strings[1];  /* TYPE_ARRAY of TYPE_STRING */
  int str_count;
  
  /* Scalar fields */
  int id;                          /* TYPE_SCALAR */
  unsigned long flags;             /* TYPE_SCALAR */
  
  /* Nested structure */
  struct GTY(()) {
    struct tree *root;             /* TYPE_POINTER */
    int depth;                     /* TYPE_SCALAR */
  } config;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  struct tree *decl_tree;          /* TYPE_POINTER */
  struct list *decl_list;          /* TYPE_POINTER */
  gty_string name;                 /* TYPE_STRING */
  
  /* Virtual method to ensure it's a C++ class */
  virtual ~declaration() {}
  
private:
  int visibility;                  /* TYPE_SCALAR */
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) root_struct {
  struct container *main_container;  /* TYPE_POINTER */
  class declaration *decl;           /* TYPE_POINTER to TYPE_LANG_STRUCT */
  
  /* Array of various types */
  struct tree * GTY((length ("%h.tree_count"))) trees[1];  /* TYPE_ARRAY */
  union node_ptr GTY((length ("%h.node_count"))) nodes[1]; /* TYPE_ARRAY */
  int tree_count;
  int node_count;
  
  /* Chain of lists */
  struct list *first_list;          /* TYPE_POINTER */
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

/* Structure that uses undefined type */
struct GTY(()) uses_undefined {
  struct undefined_struct *undef_ptr;  /* Will be TYPE_UNDEFINED */
  struct tree *defined_ptr;            /* TYPE_POINTER */
};

#endif /* TEST_GTY_INPUT_H */
