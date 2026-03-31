/* test-gty-input.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;
typedef void (*callback_func)(void *);

/* TYPE_SCALAR: Enumeration type */
enum tree_code {
  TREE_CODE_A,
  TREE_CODE_B,
  TREE_CODE_C
};

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) my_callback)(struct tree *data);

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* Skip to avoid infinite recursion */
  struct tree * GTY((tag ("0"))) tree_ptr;
  union node_ptr * GTY((tag ("1"))) node;
  int GTY((skip)) length;  /* For array length */
  my_callback callback;
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct list GTY((user)) list_t;

/* TYPE_POINTER: Pointer type definition */
typedef struct tree * GTY((pointer)) tree_ptr_t;

/* TYPE_ARRAY: Structure containing arrays */
struct GTY(()) tree {
  /* Fixed-size array of pointers */
  struct list * GTY((length ("10"))) children[10];
  
  /* Variable-length array */
  struct tree ** GTY((length ("%h.var_length"))) var_children;
  int var_length;
  
  /* Array of strings */
  gty_string_t GTY((length ("%h.num_names"))) names;
  int num_names;
  
  /* Scalar type */
  enum tree_code code;
  
  /* Regular pointer */
  struct tree *parent;
  
  /* For union discrimination */
  int type;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;
  struct list * GTY((tag ("1"))) ptr_list;
  int type;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  struct tree * GTY((skip)) decl_tree;
  gty_string_t name;
  int line;
  
  /* Array of callbacks */
  my_callback GTY((length ("3"))) callbacks[3];
  
  /* Nested structure */
  struct GTY(()) {
    int x;
    struct list *items;
  } nested;
};

#ifdef __cplusplus
}
#endif

/* More complex structure to ensure traversal */
struct GTY(()) complex_struct {
  /* Union member */
  union node_ptr choice;
  
  /* Pointer to lang struct */
  class declaration *decl;
  
  /* Self-referential pointer */
  struct complex_struct *self;
  
  /* Array of unions */
  union node_ptr GTY((length ("5"))) choices[5];
  
  /* Callback function */
  my_callback notify;
  
  /* String */
  gty_string_t description;
};

/* Root structure that ties everything together */
struct GTY(()) root_container {
  struct complex_struct *main;
  struct list *items;
  class declaration * GTY((skip)) decls[5];
  int count;
};

#endif /* TEST_GTY_INPUT_H */
