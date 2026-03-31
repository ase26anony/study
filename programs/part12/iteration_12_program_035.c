/* Test header for gengtype coverage of TYPE_* classifications */

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
  struct list * GTY((length ("%h.list_count"))) children[10];  /* TYPE_ARRAY */
  enum tree_code code;                                         /* TYPE_SCALAR */
  gty_callback_t callback;                                     /* TYPE_CALLBACK */
  int list_count;                                              /* TYPE_SCALAR */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree * GTY((atomic)) tree_ptr_t;

/* Variable-length array structure */
struct GTY(()) var_array_struct {
  int count;
  struct tree * GTY((variable_length)) items[1];  /* TYPE_ARRAY (variable) */
};

/* Nested structure for complex testing */
struct GTY(()) container {
  union node_ptr node;                    /* TYPE_UNION */
  tree_ptr_t tree_ptr;                    /* TYPE_POINTER */
  struct list * GTY((chain_next ("%h.next"))) chain;  /* TYPE_POINTER */
  gty_string_t strings[3];                /* TYPE_ARRAY of TYPE_STRING */
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  struct tree * GTY((skip)) decl_tree;    /* TYPE_POINTER */
  struct container * GTY((skip)) decl_container;  /* TYPE_POINTER */
  int decl_id;                            /* TYPE_SCALAR */
  
  /* Method to ensure class is processed */
  void GTY((callback)) process() {}
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) root_struct {
  struct container main_container;        /* TYPE_STRUCT */
  class declaration * GTY((skip)) decl;   /* TYPE_POINTER to TYPE_LANG_STRUCT */
  union node_ptr root_node;               /* TYPE_UNION */
  gty_callback_t root_callback;           /* TYPE_CALLBACK */
};

#endif /* TEST_GTY_INPUT_H */
