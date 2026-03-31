/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;

/* TYPE_SCALAR: Enumeration */
enum tree_code {
  TREE_CODE_A,
  TREE_CODE_B
};

/* TYPE_STRUCT with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list *next;          /* TYPE_POINTER */
  union node_ptr *GTY((tag ("0"))) node; /* TYPE_UNION via pointer */
  int count;                  /* TYPE_SCALAR */
};

/* TYPE_UNION with desc */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree *GTY((tag ("1"))) ptr_tree; /* TYPE_POINTER */
  struct list *GTY((tag ("2"))) ptr_list; /* TYPE_POINTER */
  int type;                   /* discriminator - TYPE_SCALAR */
};

/* TYPE_ARRAY (fixed-length) */
struct GTY(()) tree {
  struct list *children[4];   /* TYPE_ARRAY of pointers */
  enum tree_code code;        /* TYPE_SCALAR */
  const char * GTY((string)) name; /* TYPE_STRING */
};

/* TYPE_USER_STRUCT via typedef */
typedef struct GTY(()) tree user_tree;

/* TYPE_POINTER in typedef */
typedef struct list * GTY((skip)) list_ptr;

/* TYPE_CALLBACK */
typedef void (* GTY((callback)) callback_fn)(struct tree *);

/* TYPE_LANG_STRUCT (C++ class) */
class GTY((user)) declaration {
public:
  struct tree *decl_tree;     /* TYPE_POINTER */
  callback_fn fn;             /* TYPE_CALLBACK */
  int decl_id;                /* TYPE_SCALAR */
};

/* Root container structure to tie everything together */
struct GTY(()) root_container {
  struct list *first_list;    /* TYPE_POINTER */
  union node_ptr root_node;   /* TYPE_UNION (direct, not pointer) */
  user_tree *root_tree;       /* TYPE_POINTER to user struct */
  class declaration *decl;    /* TYPE_POINTER to lang struct */
  callback_fn handlers[2];    /* TYPE_ARRAY of callbacks */
};

#endif /* TEST_GTY_INPUT_H */
