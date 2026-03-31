/* Test header for gengtype coverage of switch cases in gengtype-state.cc */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_NONE,
  CODE_LIST,
  CODE_TREE,
  CODE_DECL
} tree_code GTY(());

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* Forward declarations */
struct tree;
struct list;
union node_ptr;

/* TYPE_STRUCT: Basic structure */
struct GTY(()) base_struct {
  int id;
  const char * GTY((skip)) name;  /* Non-GC pointer for contrast */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct * GTY((tag("0"))) base;
  gty_string_t description;
  int value;
} user_struct_t;

/* TYPE_POINTER: Pointer typedef */
typedef user_struct_t * GTY(()) user_ptr_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  int length;
  
  /* Fixed-size array of pointers */
  user_struct_t * GTY((length("10"))) fixed_array[10];
  
  /* Variable-length array */
  struct base_struct * GTY((length("%h.length"))) variable_array[1];
};

/* TYPE_UNION: Discriminated union */
union GTY((desc("%0.type"))) node_union {
  int type;  /* discriminator */
  struct tree * GTY((tag("1"))) tree_ptr;
  struct list * GTY((tag("2"))) list_ptr;
  user_struct_t * GTY((tag("3"))) user_ptr;
};

/* TYPE_STRUCT with nested types */
struct GTY((chain_next("%h.next"))) list {
  struct list * GTY((skip)) prev;  /* For doubly-linked list contrast */
  struct list *next;
  union node_union data;
  gty_callback_t callback;
};

/* Another structure for complex dependencies */
struct GTY(()) tree {
  tree_code code;
  gty_string_t name;
  struct list *children;
  struct tree *parent;
  struct tree * GTY((length("3"))) siblings[3];
  int depth;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY(()) declaration {
public:
  struct tree *decl_tree;
  user_struct_t *user_data;
  int line_number;
  gty_string_t filename;
  
  /* Array of callbacks */
  gty_callback_t GTY((length("5"))) handlers[5];
};

#ifdef __cplusplus
}
#endif

/* Root container that references everything */
struct GTY(()) root_container {
  struct list *main_list;
  struct tree *root_tree;
  union node_union main_union;
  user_struct_t *users[5];
  struct array_container arrays;
  class declaration *main_decl;
  gty_callback_t global_callback;
};

#endif /* TEST_GTY_INPUT_H */
