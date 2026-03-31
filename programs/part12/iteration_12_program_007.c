/* Test header to cover all gengtype type classifications */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_NONE,
  CODE_STRUCT,
  CODE_UNION,
  CODE_LAST
} gty_enum GTY((scalar));

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback_func)(void *data);

/* Forward declarations for mutual dependencies */
struct gty_tree;
struct gty_list;
union gty_node;

/* TYPE_STRUCT: Basic structure with chain_next */
struct GTY((chain_next ("%h.next"))) gty_list {
  struct gty_list * GTY((skip)) next;
  union gty_node *node;
  int GTY((skip)) count;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) gty_tree {
  struct gty_list ** GTY((length ("%h.list_count"))) children;
  int list_count;
  gty_enum code;
  gty_string name;
} gty_tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type_tag"))) gty_node {
  struct gty_tree * GTY((tag ("0"))) as_tree;
  struct gty_list * GTY((tag ("1"))) as_list;
  int type_tag;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) gty_array_container {
  /* Fixed-size array */
  struct gty_tree * GTY((length ("10"))) fixed_array[10];
  
  /* Variable-length array */
  struct gty_list ** GTY((variable_length)) var_array;
  
  /* Array of strings */
  gty_string * GTY((length ("%h.str_count"))) strings;
  int str_count;
  
  /* Multi-dimensional array */
  struct gty_tree * GTY((length ("%h.dim1 * %h.dim2"))) matrix[5][5];
  int dim1;
  int dim2;
};

/* TYPE_POINTER: Various pointer types */
typedef struct gty_tree * GTY((user)) gty_tree_ptr;
typedef struct gty_list * GTY((atomic)) gty_list_ptr;

/* Complex structure using all features */
struct GTY(()) gty_complex {
  /* Nested structure */
  struct GTY(()) {
    gty_tree_ptr tree;
    gty_list_ptr list;
  } nested;
  
  /* Union field */
  union gty_node current;
  
  /* Array of pointers */
  gty_tree_ptr * GTY((length ("%h.ptr_count"))) ptr_array;
  int ptr_count;
  
  /* Callback */
  gty_callback_func callback;
  
  /* String */
  gty_string description;
  
  /* Scalar */
  gty_enum enum_field;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" or processed specially) */
#ifdef __cplusplus
class GTY((user)) gty_cpp_class {
private:
  struct gty_tree *m_tree;
  struct gty_list *m_list;
  
public:
  gty_cpp_class() : m_tree(0), m_list(0) {}
  virtual ~gty_cpp_class() {}
  
  void set_tree(struct gty_tree *t) { m_tree = t; }
  struct gty_tree *get_tree() const { return m_tree; }
};
#endif

/* Root structure that ties everything together */
struct GTY((user)) gty_root {
  struct gty_complex *complex;
  struct gty_array_container *arrays;
  struct gty_list *list_head;
#ifdef __cplusplus
  class gty_cpp_class *cpp_obj;
#endif
  gty_callback_func root_callback;
};

#endif /* TEST_GTY_INPUT_H */
