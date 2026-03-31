/* Test header for gengtype coverage of all TYPE_* classifications */

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
typedef void (* GTY((callback)) callback_func)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string_t name;               /* TYPE_STRING */
  callback_func callback;          /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;             /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct list ** GTY((length ("%h.child_count"))) var_array;
  int child_count;
  
  /* Nested structure for additional complexity */
  struct GTY(()) inner {
    struct tree *parent;           /* TYPE_POINTER */
    int depth;                     /* TYPE_SCALAR */
  } inner_struct;
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct list * GTY(()) list_ptr_t;

/* Container structure using all types */
struct GTY(()) container {
  /* TYPE_STRUCT reference */
  struct tree root;                /* Embedded TYPE_STRUCT */
  
  /* TYPE_POINTER */
  tree_t *current;                 /* TYPE_POINTER via typedef */
  
  /* TYPE_UNION */
  union node_ptr active_node;
  
  /* TYPE_ARRAY of TYPE_UNION */
  union node_ptr GTY((length ("5"))) node_array[5];
  
  /* TYPE_ARRAY of TYPE_POINTER */
  list_ptr_t GTY((length ("%h.list_count"))) list_array;
  int list_count;
  
  /* TYPE_STRING */
  gty_string_t container_name;
  
  /* TYPE_CALLBACK */
  callback_func on_update;
  
  /* TYPE_SCALAR */
  unsigned int flags;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((operator delete)) declaration {
public:
  struct tree *decl_tree;          /* TYPE_POINTER */
  gty_string_t decl_name;          /* TYPE_STRING */
  
  /* TYPE_ARRAY in class */
  struct list * GTY((length ("3"))) decl_list[3];
  
  /* TYPE_UNION in class */
  union node_ptr decl_union;
  
private:
  int decl_id;                     /* TYPE_SCALAR */
};

#ifdef __cplusplus
}
#endif

/* Root structure that ties everything together */
struct GTY(()) root_struct {
  struct container main_container; /* TYPE_STRUCT */
  class declaration *main_decl;    /* TYPE_POINTER to TYPE_LANG_STRUCT */
  union node_ptr root_node;        /* TYPE_UNION */
  
  /* TYPE_ARRAY of different types */
  tree_t * GTY((length ("%h.tree_count"))) tree_array;
  struct list *list_array[5];      /* TYPE_ARRAY of TYPE_POINTER */
  gty_string_t string_array[3];    /* TYPE_ARRAY of TYPE_STRING */
  
  int tree_count;                  /* TYPE_SCALAR */
  
  /* TYPE_CALLBACK array */
  callback_func GTY((length ("2"))) callbacks[2];
};

/* Additional TYPE_UNDEFINED test */
/* Forward declared but never defined - will be TYPE_UNDEFINED */
struct GTY(()) undefined_struct;

/* Pointer to undefined type */
typedef struct undefined_struct * GTY(()) undefined_ptr;

#endif /* TEST_GTY_INPUT_H */
