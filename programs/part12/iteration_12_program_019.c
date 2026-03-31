/* Test header to trigger all gengtype type classifications */
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
  struct list * GTY((tag ("0"))) children[4];  /* TYPE_ARRAY of TYPE_POINTER */
  gty_string_t GTY((length ("strlen(%h.value) + 1"))) value; /* TYPE_STRING */
  struct tree *parent;             /* TYPE_POINTER */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("1"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("2"))) ptr_list;    /* TYPE_POINTER */
  int type;                                   /* TYPE_SCALAR */
};

/* TYPE_ARRAY: Structure with variable-length array */
struct GTY(()) array_container {
  int length;                                 /* TYPE_SCALAR */
  struct tree * GTY((length ("%h.length"))) elements[1]; /* TYPE_ARRAY */
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  struct tree *decl_tree;                     /* TYPE_POINTER */
  struct list *decl_list;                     /* TYPE_POINTER */
  gty_callback_t callback;                    /* TYPE_CALLBACK */
  int decl_id;                                /* TYPE_SCALAR */
};

#ifdef __cplusplus
}
#endif

/* TYPE_UNDEFINED: Forward declared incomplete type */
struct GTY(()) incomplete;

/* Complex structure using all types */
struct GTY(()) container {
  struct list *first;                         /* TYPE_POINTER */
  union node_ptr current;                     /* TYPE_UNION */
  struct array_container *arr;                /* TYPE_POINTER */
  class declaration *decl;                    /* TYPE_POINTER */
  gty_callback_t handlers[3];                 /* TYPE_ARRAY of TYPE_CALLBACK */
  struct incomplete *future;                  /* TYPE_POINTER (to TYPE_UNDEFINED) */
};

/* TYPE_UNDEFINED: Now define the incomplete type */
struct GTY(()) incomplete {
  struct container *owner;                    /* TYPE_POINTER */
  gty_string_t type_name;                     /* TYPE_STRING */
};

#endif /* TEST_GTY_INPUT_H */
