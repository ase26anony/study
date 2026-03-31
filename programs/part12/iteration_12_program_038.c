/* Test header for gengtype coverage of TYPE_* classifications */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_A,
  CODE_B,
  CODE_C
} tree_code GTY(());

/* TYPE_STRING: String type */
typedef const char * GTY((string)) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) callback_func)(void *data);

/* TYPE_STRUCT: Basic structure */
struct GTY(()) tree {
  tree_code code;              /* TYPE_SCALAR */
  string_type name;            /* TYPE_STRING */
  callback_func cb;            /* TYPE_CALLBACK */
  int value;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct tree * GTY((skip)) ptr_tree;  /* TYPE_POINTER with skip */
  int data;
} user_struct_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) node_union {
  struct tree * GTY((tag ("0"))) ptr_tree;    /* TYPE_POINTER */
  user_struct_t * GTY((tag ("1"))) ptr_user;  /* TYPE_POINTER */
  int type;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) list {
  struct tree * GTY((length ("%h.count"))) items[10];  /* Fixed array */
  struct list * GTY((chain_next ("%h.next"))) next;    /* Linked list */
  int count;
};

/* Variable length array */
struct GTY(()) vla_struct {
  int length;
  struct tree ** GTY((variable_length)) array;  /* Variable length array */
};

/* TYPE_POINTER: Typedef for pointer */
typedef struct tree * GTY(()) tree_ptr;

/* Nested structure with multiple pointer types */
struct GTY(()) container {
  tree_ptr primary;                    /* TYPE_POINTER via typedef */
  union node_union choice;             /* TYPE_UNION */
  struct list * GTY((atomic)) items;   /* TYPE_POINTER with atomic */
  struct container *parent;            /* TYPE_POINTER (implicit GTY) */
  user_struct_t user;                  /* TYPE_USER_STRUCT */
};

/* For TYPE_LANG_STRUCT - C++ class */
#ifdef __cplusplus
class GTY(()) lang_class {
public:
  struct container * GTY((reorder ("container_reorder"))) cont;
  struct tree *current;
  
  lang_class() : cont(0), current(0) {}
  
private:
  int private_data;
};
#endif

/* Root structure that references everything */
struct GTY(()) root_struct {
  struct container *main_container;
  struct list *item_list;
  struct vla_struct *vla;
  #ifdef __cplusplus
  class lang_class *lang_obj;
  #endif
};

#endif /* TEST_GTY_INPUT_H */
