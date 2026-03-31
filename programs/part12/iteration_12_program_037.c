/* Test file to cover all gengtype-state.cc switch cases */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_A,
  CODE_B,
  CODE_C
} tree_code GTY(());

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_func)(void *data);

/* Forward declarations */
struct tree_s;
struct list_s;
union node_u;

/* TYPE_STRUCT: Basic structure */
struct GTY(()) base_struct {
  int id;                          /* TYPE_SCALAR */
  gty_string name;                 /* TYPE_STRING */
  struct tree_s * GTY((skip)) tree_ptr; /* TYPE_POINTER (skip marker) */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct *base;        /* TYPE_POINTER */
  gty_callback_func callback;      /* TYPE_CALLBACK */
  int data;
} user_struct_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_struct {
  struct base_struct * GTY((length ("%h.count"))) items[10]; /* Fixed array */
  int count;
  
  /* Variable length array */
  user_struct_t * GTY((variable_length)) var_items;
  int var_count;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_u {
  int type;                        /* Discriminator - TYPE_SCALAR */
  struct tree_s * GTY((tag ("TYPE_TREE"))) as_tree;  /* TYPE_POINTER */
  struct list_s * GTY((tag ("TYPE_LIST"))) as_list;  /* TYPE_POINTER */
  user_struct_t * GTY((tag ("TYPE_USER"))) as_user;  /* TYPE_POINTER */
};

/* TYPE_STRUCT with chain_next for linked list */
struct GTY((chain_next ("%h.next"))) list_s {
  struct list_s *next;             /* TYPE_POINTER with chain_next */
  union node_u data;               /* TYPE_UNION */
  int value;                       /* TYPE_SCALAR */
};

/* Another structure with nested pointers */
struct GTY(()) tree_s {
  tree_code code;                  /* TYPE_SCALAR (enum) */
  gty_string label;                /* TYPE_STRING */
  
  /* Array of pointers */
  struct list_s * GTY((length ("%h.list_count"))) lists[5]; /* TYPE_ARRAY */
  int list_count;
  
  /* Pointer to array structure */
  struct array_struct *arrays;     /* TYPE_POINTER */
  
  /* For TYPE_POINTER in typedef */
  struct tree_s *parent;           /* TYPE_POINTER */
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" or processed specially) */
#ifdef __cplusplus
class GTY(()) lang_class {
private:
  struct tree_s *root;             /* TYPE_POINTER */
  user_struct_t *user_data;        /* TYPE_POINTER */
  
public:
  lang_class() : root(0), user_data(0) {}
  virtual ~lang_class() {}
  
  void set_root(struct tree_s *r) { root = r; }
};
#endif

/* Root structure that ties everything together */
struct GTY(()) root_container {
  struct tree_s *main_tree;        /* TYPE_POINTER */
  struct list_s *head;             /* TYPE_POINTER */
  union node_u current_node;       /* TYPE_UNION */
  struct array_struct arrays;      /* TYPE_STRUCT (embedded) */
  
  /* For TYPE_UNDEFINED coverage - forward declared pointer */
  struct undefined_struct *undef_ptr; /* Will be TYPE_UNDEFINED */
  
#ifdef __cplusplus
  class lang_class *lang_obj;      /* TYPE_POINTER to TYPE_LANG_STRUCT */
#endif
};

/* TYPE_POINTER in typedef */
typedef struct tree_s * GTY(()) tree_ptr_t;

/* Another pointer type with different attributes */
typedef union node_u * GTY((atomic)) atomic_node_ptr;

#endif /* TEST_GTY_INPUT_H */
