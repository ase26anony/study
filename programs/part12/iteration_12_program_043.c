/* Test header to trigger all gengtype type classifications */
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
typedef const char * GTY((string)) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) callback_func)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  string_type name;                /* TYPE_STRING */
  callback_func cb;                /* TYPE_CALLBACK */
};

/* Another structure for dependencies */
struct GTY(()) tree {
  tree_code code;                  /* TYPE_SCALAR */
  struct list *children;           /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("5"))) fixed_array[5];
  
  /* TYPE_ARRAY: Variable-length array */
  struct list ** GTY((length ("%h.child_count"))) var_array;
  int child_count;
  
  /* For union discrimination */
  int node_type;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct tree tree_t GTY(());

/* More complex structure using all previous types */
struct GTY(()) complex_struct {
  tree_t *root;                     /* TYPE_POINTER to TYPE_USER_STRUCT */
  union node_ptr current;           /* TYPE_UNION */
  struct list * GTY((chain_next ("%h.next"))) head;  /* TYPE_POINTER */
  
  /* Nested structure */
  struct GTY(()) nested {
    int value;
    struct tree *link;
  } nested_item;
  
  /* Array of unions */
  union node_ptr GTY((length ("10"))) union_array[10];
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((operator delete)) declaration {
public:
  struct tree *decl_tree;           /* TYPE_POINTER */
  struct complex_struct *decl_data; /* TYPE_POINTER */
  int decl_id;                      /* TYPE_SCALAR */
  
  /* Method to ensure class is processed */
  virtual void dummy() {}
};

#ifdef __cplusplus
}
#endif

/* Container structure holding everything */
struct GTY(()) container {
  struct complex_struct *data;      /* TYPE_POINTER */
  class declaration *decl;          /* TYPE_POINTER to TYPE_LANG_STRUCT */
  union node_ptr active_node;       /* TYPE_UNION */
  
  /* Two-dimensional array */
  struct list * GTY((length ("%h.rows * %h.cols"))) matrix;
  int rows;
  int cols;
  
  /* Callback array */
  callback_func GTY((length ("%h.callback_count"))) callbacks[3];
  int callback_count;
};

/* Root type that references everything */
extern struct container * GTY(()) root_container;

#endif /* TEST_GTY_INPUT_H */
