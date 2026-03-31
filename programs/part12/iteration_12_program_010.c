/* Test file to cover all type classifications in gengtype-state.cc */
#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* Include gtype-desc.h for GTY macro if not already defined */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* TYPE_SCALAR: Enumeration type */
typedef enum GTY(()) tree_code {
  TREE_CODE_ERROR,
  TREE_CODE_IDENTIFIER,
  TREE_CODE_DECL,
  TREE_CODE_TYPE,
  TREE_CODE_CONSTANT
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic structure */
struct GTY(()) base_struct {
  int GTY((skip)) scalar_field;      /* TYPE_SCALAR */
  gty_string GTY((tag("string_field"))) string_field;  /* TYPE_STRING */
  gty_callback callback_field;       /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct * GTY((tag("base_ptr"))) ptr_to_base;  /* TYPE_POINTER */
  int data;
} user_struct_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  struct base_struct * GTY((length("5"))) fixed_array[5];
  
  /* Variable-length array */
  struct user_struct * GTY((length("%h.var_len"))) *var_array;
  int var_len;
  
  /* Nested array */
  int GTY((skip)) matrix[3][3];
};

/* TYPE_UNION: Discriminated union */
union GTY((desc("%0.union_tag"))) discriminated_union {
  struct base_struct * GTY((tag("ptr_base"))) as_base;
  struct array_container * GTY((tag("ptr_array"))) as_array;
  user_struct_t * GTY((tag("ptr_user"))) as_user;
  int union_tag;
};

/* TYPE_POINTER: Special pointer type with attributes */
typedef struct base_struct * GTY((chain_next("%h.next"))) gty_special_ptr;

/* Linked list structure using chain_next */
struct GTY((chain_next("%h.next"))) linked_list {
  struct linked_list *next;
  union discriminated_union data;
  int id;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Pointer to union */
  union discriminated_union * GTY((tag("union_ptr"))) union_ptr;
  
  /* Array of linked lists */
  struct linked_list * GTY((length("%h.list_count"))) *list_array;
  int list_count;
  
  /* Multi-dimensional pointer array */
  struct base_struct * GTY((length("%h.rows * %h.cols"))) **matrix_ptr;
  int rows;
  int cols;
  
  /* Self-referential pointer */
  struct complex_nested *self;
};

/* TYPE_LANG_STRUCT: C++ class definition */
#ifdef __cplusplus
class GTY(()) lang_class {
private:
  struct linked_list * GTY((tag("list_member"))) list_member;
  struct complex_nested *complex_data;
  
public:
  lang_class() : list_member(0), complex_data(0) {}
  virtual ~lang_class() {}
  
  void set_list(struct linked_list *list) { list_member = list; }
  struct linked_list *get_list() { return list_member; }
};
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct base_struct base_instance;
  user_struct_t user_instance;
  struct array_container array_instance;
  union discriminated_union union_instance;
  struct linked_list *list_head;
  struct complex_nested *complex_root;
  
  #ifdef __cplusplus
  class lang_class *lang_obj;
  #endif
  
  /* String array */
  gty_string GTY((length("%h.string_count"))) *string_array;
  int string_count;
  
  /* Callback array */
  gty_callback GTY((length("%h.callback_count"))) *callback_array;
  int callback_count;
};

/* Global variable to ensure types are referenced */
extern struct root_container * GTY((tag("global_root"))) global_root;

#endif /* TEST_GTY_INPUT_H */
