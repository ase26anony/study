/* Test header for gengtype coverage of TYPE_* classifications */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Forward declarations */
struct tree;
struct list;
union node_ptr;
class declaration;
enum tree_code;

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
  gty_callback_t callback;         /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree tree_t;

/* TYPE_STRUCT with nested types */
struct GTY(()) tree {
  tree_t *parent;                  /* TYPE_POINTER */
  struct list *children;           /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  tree_t * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct list ** GTY((length ("%h.child_count"))) var_array;
  int child_count;                 /* TYPE_SCALAR */
  
  enum tree_code code;             /* TYPE_SCALAR */
  
  /* Discriminator for union */
  int node_type;                   /* TYPE_SCALAR */
};

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.node_type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;    /* TYPE_POINTER */
  int node_type;                              /* TYPE_SCALAR */
};

/* TYPE_STRUCT containing union */
struct GTY(()) container {
  union node_ptr item;             /* TYPE_UNION */
  struct container *next;          /* TYPE_POINTER */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct tree * GTY((user)) tree_ptr;

/* Complex structure using all features */
struct GTY(()) complex_struct {
  /* Chain linking */
  struct complex_struct * GTY((chain_next ("%h.next_chain"))) next_chain;
  struct complex_struct * GTY((chain_prev ("%h.prev_chain"))) prev_chain;
  
  /* Array of unions */
  union node_ptr GTY((length ("%h.union_count"))) union_array[5];
  int union_count;                 /* TYPE_SCALAR */
  
  /* Pointer to array */
  tree_t ** GTY((length ("%h.ptr_count"))) ptr_array;
  int ptr_count;                   /* TYPE_SCALAR */
  
  /* Nested structure */
  struct {
    tree_t *nested_tree;           /* TYPE_POINTER */
    int nested_scalar;             /* TYPE_SCALAR */
  } GTY((skip)) nested;
  
  /* Atomic field (non-pointer) */
  int GTY((atomic)) atomic_field;  /* TYPE_SCALAR */
};

/* TYPE_UNDEFINED: Forward declared but never defined */
struct GTY(()) undefined_struct;

/* Extern C block for C++ class */
#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_LANG_STRUCT: C++ class definition */
class GTY((user)) declaration {
private:
  tree_t *decl_tree;               /* TYPE_POINTER */
  gty_string_t decl_name;          /* TYPE_STRING */
  
public:
  declaration() : decl_tree(0), decl_name(0) {}
  void set_tree(tree_t *t) { decl_tree = t; }
};

#ifdef __cplusplus
}
#endif

/* Template structure (C++ feature) */
template<typename T>
class GTY((user)) template_container {
  T * GTY((skip)) item;
public:
  template_container() : item(0) {}
};

#endif /* TEST_GTY_INPUT_H */
