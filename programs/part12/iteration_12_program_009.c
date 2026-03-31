/* test-gty-input.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

#include <stddef.h>

/* Forward declarations for type dependencies */
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

/* TYPE_STRING: String type with GTY markup */
typedef const char * GTY((string)) gty_string_t;

/* TYPE_CALLBACK: Function pointer callback type */
typedef void (* GTY((callback)) gty_callback_t)(void *data, int value);

/* TYPE_STRUCT: Basic structure with chain_next for linked list */
struct GTY((chain_next ("%h.next"))) list {
  struct list * GTY((skip)) next;  /* TYPE_POINTER */
  struct tree *tree_ptr;           /* TYPE_POINTER */
  union node_ptr *node;            /* TYPE_POINTER to TYPE_UNION */
  int data;                        /* TYPE_SCALAR */
  gty_string_t name;               /* TYPE_STRING */
  gty_callback_t callback;         /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) tree {
  struct list ** GTY((length ("%h.list_count"))) list_array;  /* TYPE_ARRAY of TYPE_POINTER */
  int list_count;                                             /* TYPE_SCALAR */
  enum tree_code code;                                        /* TYPE_SCALAR */
  struct tree * GTY((tag ("0"))) left;                       /* TYPE_POINTER */
  struct tree * GTY((tag ("1"))) right;                      /* TYPE_POINTER */
  unsigned char flags;                                        /* TYPE_SCALAR */
} tree_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_ptr {
  struct tree * GTY((tag ("0"))) ptr_tree;  /* TYPE_POINTER */
  struct list * GTY((tag ("1"))) ptr_list;  /* TYPE_POINTER */
  int type;                                 /* TYPE_SCALAR - discriminant */
};

/* TYPE_ARRAY: Structure with embedded array */
struct GTY(()) array_container {
  tree_t * GTY((length ("10"))) fixed_array[10];      /* TYPE_ARRAY fixed size */
  struct list * GTY((variable_length)) var_array[1];  /* TYPE_ARRAY variable length */
  int var_length;                                     /* TYPE_SCALAR */
};

/* TYPE_POINTER: Typedef for pointer type */
typedef tree_t * GTY((atomic)) tree_ptr_t;

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_struct {
  union node_ptr union_field;                /* TYPE_UNION */
  struct array_container array_field;        /* TYPE_STRUCT containing TYPE_ARRAY */
  tree_ptr_t typedef_ptr;                    /* TYPE_POINTER via typedef */
  struct complex_struct * GTY((skip)) self;  /* TYPE_POINTER to self */
};

/* Additional structure for TYPE_STRUCT coverage */
struct GTY(()) another_struct {
  struct list *head;                         /* TYPE_POINTER */
  struct tree *root;                         /* TYPE_POINTER */
  gty_string_t *string_array[5];             /* TYPE_ARRAY of TYPE_STRING */
};

#endif /* TEST_GTY_INPUT_H */
