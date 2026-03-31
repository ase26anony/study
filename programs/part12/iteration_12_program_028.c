/* Test header for gengtype coverage of all type classifications */

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
  gty_callback_t callback;         /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  enum tree_code code;             /* TYPE_SCALAR */
  struct list * GTY((tag ("0"))) children;  /* TYPE_POINTER */
  
  /* TYPE_ARRAY: Fixed-size array of pointers */
  struct list * GTY((length ("10"))) fixed_array[10];
  
  /* TYPE_ARRAY: Variable-length array */
  struct tree * GTY((variable_length)) var_array[1];
  
  int length;  /* For variable_length array */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("1"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("2"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR */
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY((user)) declaration {
public:
  tree_t * GTY((skip)) decl_tree;           /* TYPE_POINTER */
  struct list * GTY((chain_next ("%h.next"))) decl_list;  /* TYPE_POINTER */
  
  /* Nested structure inside class */
  struct GTY(()) nested {
    tree_t *nested_ptr;                     /* TYPE_POINTER */
  } nested_data;
  
  /* Array inside class */
  node_ptr GTY((length ("5"))) nodes[5];    /* TYPE_ARRAY of TYPE_UNION */
  
  /* String member */
  gty_string_t decl_name;                   /* TYPE_STRING */
};

#ifdef __cplusplus
}
#endif

/* TYPE_POINTER: Typedef for pointer type */
typedef tree_t * GTY((user)) tree_ptr_t;

/* Complex structure using all types */
struct GTY(()) complex_struct {
  /* TYPE_STRUCT nested field */
  struct GTY(()) inner {
    tree_ptr_t inner_ptr;                   /* TYPE_POINTER */
    int inner_data;                         /* TYPE_SCALAR */
  } inner_field;
  
  /* TYPE_UNION field */
  union GTY((desc ("%0.utype"))) {
    tree_t *u_tree;                         /* TYPE_POINTER */
    struct list *u_list;                    /* TYPE_POINTER */
    char u_char;                            /* TYPE_SCALAR */
  } union_field;
  
  int utype;  /* discriminator for union */
  
  /* TYPE_ARRAY of different types */
  declaration * GTY((length ("%h.decl_count"))) declarations[1];  /* TYPE_ARRAY of TYPE_LANG_STRUCT */
  int decl_count;
  
  /* TYPE_CALLBACK field */
  gty_callback_t handler;                   /* TYPE_CALLBACK */
  
  /* Chain pointer */
  struct complex_struct * GTY((chain_next ("%h.chain"))) chain;  /* TYPE_POINTER */
};

/* Root structure that references everything */
struct GTY(()) root {
  tree_t *root_tree;                        /* TYPE_POINTER */
  struct list *root_list;                   /* TYPE_POINTER */
  declaration *root_decl;                   /* TYPE_POINTER to TYPE_LANG_STRUCT */
  struct complex_struct *complex;           /* TYPE_POINTER */
  node_ptr root_union;                      /* TYPE_UNION */
  
  /* TYPE_ARRAY with callback */
  gty_callback_t GTY((length ("3"))) callbacks[3];  /* TYPE_ARRAY of TYPE_CALLBACK */
  
  /* String array */
  gty_string_t GTY((length ("2"))) strings[2];      /* TYPE_ARRAY of TYPE_STRING */
};

#endif /* TEST_GTY_INPUT_H */
