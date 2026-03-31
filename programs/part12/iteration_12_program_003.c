/* Test file to cover all gengtype-state.cc type classification cases */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_NONE,
  CODE_STRUCT,
  CODE_UNION,
  CODE_ARRAY
} tree_code GTY(());

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) tree_traverse_fn)(void *node, void *data);

/* Basic structure for TYPE_STRUCT */
struct GTY((chain_next ("%h.next"))) tree_node {
  struct tree_node * GTY((skip)) next;  /* TYPE_POINTER */
  tree_code code;                       /* TYPE_SCALAR */
  gty_string name;                      /* TYPE_STRING */
  int value;                            /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: Typedef of a structure */
typedef struct tree_node tree GTY(());

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) tree_container {
  tree * GTY((length ("%h.count"))) children[10];  /* TYPE_ARRAY of TYPE_POINTER */
  int count;                                       /* TYPE_SCALAR */
  tree * GTY((variable_length)) extra_children[];  /* Variable length array */
};

/* Discriminated union for TYPE_UNION */
struct GTY(()) union_discriminator {
  int type;  /* TYPE_SCALAR - discriminator field */
};

union GTY((desc ("%0.type"))) tree_union {
  struct union_discriminator GTY((skip)) desc;  /* For desc tag */
  tree *ptr_tree;                               /* TYPE_POINTER */
  struct tree_container *ptr_container;         /* TYPE_POINTER */
  int int_value;                                /* TYPE_SCALAR */
};

/* Complex structure with nested types */
struct GTY(()) complex_structure {
  tree *root;                                   /* TYPE_POINTER */
  struct tree_container container;              /* TYPE_STRUCT */
  union tree_union data;                        /* TYPE_UNION */
  tree_traverse_fn traverse;                    /* TYPE_CALLBACK */
  
  /* Nested anonymous union */
  union {
    tree *left;                                 /* TYPE_POINTER */
    tree *right;                                /* TYPE_POINTER */
  } GTY((tag ("0"))) children;
  
  /* Pointer to array */
  tree ** GTY((length ("%h.ptr_count"))) ptr_array;  /* TYPE_POINTER to TYPE_ARRAY */
  int ptr_count;                                     /* TYPE_SCALAR */
};

/* Linked list using chain_next */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) double_linked_list {
  struct double_linked_list *next;              /* TYPE_POINTER */
  struct double_linked_list *prev;              /* TYPE_POINTER */
  tree *data;                                   /* TYPE_POINTER */
  int id;                                       /* TYPE_SCALAR */
};

/* Structure with callback array */
struct GTY(()) callback_container {
  tree_traverse_fn GTY((length ("%h.callback_count"))) callbacks[5];  /* TYPE_ARRAY of TYPE_CALLBACK */
  int callback_count;                                                  /* TYPE_SCALAR */
};

#ifdef __cplusplus

/* TYPE_LANG_STRUCT: C++ class definition */
class GTY((operator delete)) lang_class {
private:
  tree *m_tree;                                 /* TYPE_POINTER */
  gty_string m_name;                            /* TYPE_STRING */
  
public:
  lang_class() : m_tree(0), m_name("default") {}
  virtual ~lang_class() {}
  
  void set_tree(tree *t) { m_tree = t; }        /* TYPE_POINTER access */
  tree* get_tree() const { return m_tree; }
  
  /* Virtual method table pointer should be skipped */
  virtual void traverse() = 0;
};

/* Derived class */
class GTY((operator delete)) derived_class : public lang_class {
private:
  struct tree_container *m_container;           /* TYPE_POINTER */
  
public:
  derived_class() : m_container(0) {}
  ~derived_class() override {}
  
  void traverse() override {
    /* Implementation */
  }
};

#endif /* __cplusplus */

/* Root structure that references everything */
struct GTY(()) root_container {
  tree *root_tree;                              /* TYPE_POINTER */
  struct complex_structure complex;             /* TYPE_STRUCT */
  struct double_linked_list *list_head;         /* TYPE_POINTER */
  struct callback_container callbacks;          /* TYPE_STRUCT */
  
#ifdef __cplusplus
  class lang_class *lang_obj;                   /* TYPE_POINTER to TYPE_LANG_STRUCT */
#endif
  
  /* Self-referential pointer */
  struct root_container *self;                  /* TYPE_POINTER */
};

/* TYPE_UNDEFINED: Forward declaration that will be processed */
struct GTY(()) forward_declared_struct;

/* Complete the forward declared structure */
struct GTY(()) forward_declared_struct {
  tree *content;                                /* TYPE_POINTER */
  struct forward_declared_struct *next;         /* TYPE_POINTER */
};

/* Another structure that uses the forward declared one */
struct GTY(()) uses_forward_decl {
  struct forward_declared_struct *fwd_ptr;      /* TYPE_POINTER */
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_INPUT_H */
