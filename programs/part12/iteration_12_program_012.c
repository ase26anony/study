/* Test file to cover all gengtype-state.cc switch cases */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations for type dependencies */
struct tree;
struct list;
union node_ptr;
class declaration;

/* TYPE_SCALAR: Enumeration type */
typedef enum {
  TREE_CODE_INT,
  TREE_CODE_FLOAT,
  TREE_CODE_STRING
} tree_code GTY(());

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) tree_callback)(struct tree *t);

/* TYPE_STRING: String type */
typedef const char * GTY((string)) tree_string;

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  struct tree *tree_ptr;           /* TYPE_POINTER */
  union node_ptr *node;            /* TYPE_POINTER to TYPE_UNION */
  int data;                        /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) children[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree ** GTY((length ("%h.child_count"))) var_children;
  
  tree_code code;                  /* TYPE_SCALAR */
  tree_string name;                /* TYPE_STRING */
  tree_callback callback;          /* TYPE_CALLBACK */
  int child_count;                 /* TYPE_SCALAR */
  unsigned int flags;              /* TYPE_SCALAR */
} tree_user;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct list * GTY(()) list_ptr;

/* Complex structure using all types */
struct GTY(()) complex_struct {
  /* Nested structures */
  struct {
    tree_user *current;            /* TYPE_POINTER */
    list_ptr head;                 /* TYPE_POINTER (typedef) */
  } GTY(()) state;
  
  /* Array of unions */
  union node_ptr GTY(()) nodes[5]; /* TYPE_ARRAY of TYPE_UNION */
  
  /* Pointer to array */
  tree_user ** GTY((length ("%h.size"))) tree_array;
  
  /* String array */
  const char * GTY((string)) names[3]; /* TYPE_ARRAY of TYPE_STRING */
  
  int size;                        /* TYPE_SCALAR */
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY((user)) declaration {
  tree_user *decl_tree;            /* TYPE_POINTER */
  complex_struct *info;            /* TYPE_POINTER */
  
public:
  declaration() : decl_tree(0), info(0) {}
  virtual ~declaration() {}
  
  void set_tree(tree_user *t) { decl_tree = t; }
};
#endif

/* Root structure that ties everything together */
struct GTY(()) root_container {
  struct list *first_list;         /* TYPE_POINTER */
  tree_user *main_tree;            /* TYPE_POINTER */
  struct complex_struct *complex;  /* TYPE_POINTER */
  
  #ifdef __cplusplus
  class declaration *decl;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
  #endif
  
  /* Self-referential pointer */
  struct root_container * GTY((skip)) self;
};

#endif /* TEST_GTY_INPUT_H */
