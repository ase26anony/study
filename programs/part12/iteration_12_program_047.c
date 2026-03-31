/* Test file to cover all gengtype-state.cc type classifications */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_VOID,
  TREE_INTEGER,
  TREE_REAL,
  TREE_STRING,
  TREE_COMPLEX,
  TREE_VECTOR
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_t)(void *data);

/* Forward declarations */
struct tree_node;
struct list_node;
union node_union;

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) list_node {
  struct list_node * GTY((skip)) next;  /* Skip to avoid infinite recursion */
  struct tree_node * GTY((tag ("0"))) tree_ptr;
  union node_union * GTY((tag ("1"))) union_ptr;
  int GTY((skip)) data;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree_node {
  tree_code code;                     /* TYPE_SCALAR */
  string_t GTY((length ("strlen(%h.name) + 1"))) name;  /* TYPE_STRING */
  struct list_node * GTY((length ("%h.list_len"))) children[10]; /* TYPE_ARRAY */
  int list_len;
  gty_callback_t callback;            /* TYPE_CALLBACK */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) node_union {
  struct tree_node * GTY((tag ("0"))) as_tree;
  struct list_node * GTY((tag ("1"))) as_list;
  int type_tag;
};

/* TYPE_POINTER: Typedef for pointer type */
typedef tree_t * GTY(()) tree_ptr_t;

/* Array of pointers type */
typedef struct list_node * GTY((length ("%h.ptr_count"))) ptr_array_t[5];

/* Nested structure with all features */
struct GTY(()) complex_struct {
  tree_ptr_t root;                    /* TYPE_POINTER */
  union node_union current;           /* TYPE_UNION */
  ptr_array_t pointers;               /* TYPE_ARRAY of TYPE_POINTER */
  int ptr_count;
  string_t description;               /* TYPE_STRING */
  gty_callback_t handlers[3];         /* TYPE_ARRAY of TYPE_CALLBACK */
};

/* Variable length array structure */
struct GTY(()) var_len_struct {
  int count;
  struct tree_node * GTY((variable_length)) items[1];
};

#ifdef __cplusplus

/* TYPE_LANG_STRUCT: C++ class definition */
class GTY(()) declaration {
private:
  tree_t *decl_tree;
  string_t mangled_name;
  
public:
  declaration() : decl_tree(0), mangled_name(0) {}
  void set_tree(tree_t *t) { decl_tree = t; }
  tree_t *get_tree() { return decl_tree; }
};

/* Template class with GTY */
template<typename T>
class GTY(()) gty_container {
  T * GTY((skip)) data;
  int size;
  
public:
  gty_container() : data(0), size(0) {}
};

#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_INPUT_H */
