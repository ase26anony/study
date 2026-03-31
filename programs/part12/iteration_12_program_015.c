/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

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
typedef void (* GTY((callback)) gty_callback_fn)(void *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  int data;                        /* TYPE_SCALAR */
  gty_string_t name;               /* TYPE_STRING */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;             /* TYPE_SCALAR */
  struct list * GTY((tag ("0"))) children[4];  /* TYPE_ARRAY of TYPE_POINTER */
  gty_string_t GTY((length ("strlen(%h.value) + 1"))) value; /* TYPE_STRING */
  unsigned int count;              /* TYPE_SCALAR */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("1"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("2"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* Another structure using the union */
struct GTY(()) container {
  union node_ptr GTY((skip)) item;            /* TYPE_UNION */
  gty_callback_fn callback;                   /* TYPE_CALLBACK */
  struct tree * GTY((chain_next ("%h.next_tree"))) next_tree; /* TYPE_POINTER */
};

/* Variable length array structure */
struct GTY(()) var_array {
  int length;                                 /* TYPE_SCALAR */
  struct list * GTY((length ("%h.length"))) items[1]; /* TYPE_ARRAY */
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  struct tree * GTY((skip)) decl_tree;        /* TYPE_POINTER */
  gty_string_t filename;                      /* TYPE_STRING */
  int line;                                   /* TYPE_SCALAR */
  
  /* Method to ensure class is processed */
  void GTY((callback)) register_callback(gty_callback_fn fn);
};

#ifdef __cplusplus
}
#endif

/* Root structure that references many types */
struct GTY(()) root_struct {
  struct container * GTY((skip)) first;       /* TYPE_POINTER */
  class declaration * GTY((skip)) decl;       /* TYPE_POINTER (to TYPE_LANG_STRUCT) */
  union node_ptr GTY((skip)) root_node;       /* TYPE_UNION */
  struct var_array * GTY((skip)) var_array;   /* TYPE_POINTER */
  gty_callback_fn callbacks[3];               /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

/* Pointer to undefined type */
typedef struct undefined_struct * GTY((skip)) undefined_ptr;

#endif /* TEST_GTY_INPUT_H */
