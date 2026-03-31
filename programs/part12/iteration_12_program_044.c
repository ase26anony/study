/* Test header for gengtype coverage of type state writing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_STRUCT: Basic structure with GC marking */
struct GTY(()) base_struct {
  int id;
  const char *name;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct *base;  /* TYPE_POINTER */
  int value;
} user_struct_t;

/* TYPE_UNION: Discriminated union with pointers */
union GTY((desc ("%0.kind"))) my_union {
  struct base_struct * GTY((tag ("0"))) ptr_base;  /* TYPE_POINTER */
  user_struct_t * GTY((tag ("1"))) ptr_user;       /* TYPE_POINTER */
  int kind;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  struct base_struct * GTY((length ("5"))) fixed_array[5];
  
  /* Variable-length array */
  user_struct_t ** GTY((length ("%h.count"))) var_array;
  int count;
  
  /* Nested array */
  struct GTY(()) nested {
    int data;
    struct base_struct *next;
  } nested_array[3];
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct base_struct * GTY(()) base_ptr;

/* Linked list structure for chain_next/chain_prev */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
  struct linked_node *next;
  struct linked_node *prev;
  struct base_struct *data;
  union my_union optional;
};

/* TYPE_STRING: String field */
struct GTY(()) string_container {
  const char * GTY((string)) str_field;
  char * GTY((string)) dynamic_str;
};

/* TYPE_SCALAR: Enumeration type */
enum scalar_enum {
  ENUM_A,
  ENUM_B,
  ENUM_C
};

struct GTY(()) scalar_container {
  enum scalar_enum kind;      /* TYPE_SCALAR */
  int value;                  /* TYPE_SCALAR */
  unsigned long long big_num; /* TYPE_SCALAR */
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) callback_func)(void *data, int value);

struct GTY(()) callback_container {
  callback_func handler;
  void *user_data;
};

/* Complex nested structure to ensure traversal */
struct GTY(()) root_container {
  struct linked_node *list_head;
  struct array_container arrays;
  struct string_container strings;
  struct scalar_container scalars;
  struct callback_container callback;
  union my_union variant;
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;

/* Self-referential structure */
struct GTY(()) tree_node {
  int id;
  struct tree_node *left;    /* TYPE_POINTER */
  struct tree_node *right;   /* TYPE_POINTER */
  struct base_struct *data;  /* TYPE_POINTER */
  struct tree_node *children[4];  /* TYPE_ARRAY of TYPE_POINTER */
};

#endif /* TEST_GTY_INPUT_H */
