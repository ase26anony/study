/* Test header for gengtype coverage of type state writing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_STRUCT: Basic structure with GC-tagged pointers */
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
union GTY((desc ("%0.type"))) tagged_union {
  struct base_struct * GTY((tag ("0"))) ptr_base;  /* TYPE_POINTER */
  user_struct_t * GTY((tag ("1"))) ptr_user;       /* TYPE_POINTER */
  int type;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  struct base_struct * GTY((length ("10"))) fixed_array[10];
  
  /* Variable-length array */
  user_struct_t ** GTY((length ("%h.var_len"))) var_array;
  int var_len;
  
  /* Nested array */
  tagged_union GTY((length ("5"))) union_array[5];
};

/* Linked list structure for chain_next/chain_prev */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
  struct linked_node *next;
  struct linked_node *prev;
  struct base_struct *data;
  int index;
};

/* TYPE_STRING: String fields */
struct GTY(()) string_container {
  const char * GTY((string)) str1;
  char * GTY((string)) str2;
  const char *plain_cstring;  /* Will be treated differently */
};

/* TYPE_SCALAR: Enumeration type */
enum color { RED, GREEN, BLUE };

struct GTY(()) scalar_container {
  enum color color;           /* TYPE_SCALAR */
  int count;                  /* TYPE_SCALAR */
  float value;                /* TYPE_SCALAR */
  unsigned long flags;        /* TYPE_SCALAR */
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) callback_func)(void *data, int value);

struct GTY(()) callback_container {
  callback_func handler;
  void *user_data;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_root {
  struct array_container *arrays;      /* TYPE_POINTER */
  struct linked_node *list_head;       /* TYPE_POINTER */
  struct string_container *strings;    /* TYPE_POINTER */
  struct scalar_container scalars;     /* TYPE_STRUCT */
  struct callback_container callback;  /* TYPE_STRUCT */
  union tagged_union current;          /* TYPE_UNION */
  
  /* Self-referential pointer */
  struct complex_root * GTY((skip)) self_ptr;
  
  /* Array of pointers to different types */
  void * GTY((atomic)) generic_ptrs[4];
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

/* Simple C++-like structure that gengtype will treat as TYPE_LANG_STRUCT */
class GTY(()) lang_class {
public:
  struct tree_node *child;     /* TYPE_POINTER */
  struct base_struct *base;    /* TYPE_POINTER */
  int lang_specific;
};

#ifdef __cplusplus
}
#endif

/* Tree structure with multiple pointer types */
struct GTY(()) tree_node {
  struct tree_node *left;      /* TYPE_POINTER */
  struct tree_node *right;     /* TYPE_POINTER */
  struct base_struct *data;    /* TYPE_POINTER */
  class lang_class *lang_info; /* TYPE_POINTER to LANG_STRUCT */
  
  /* Array of pointers within tree */
  struct tree_node ** GTY((length ("%h.child_count"))) children;
  int child_count;
  
  /* String data */
  const char * GTY((string)) node_name;
  
  /* Scalar discriminant */
  enum { NODE_TYPE_A, NODE_TYPE_B } node_type;
};

/* Container with all types */
struct GTY(()) master_container {
  /* All structure types */
  struct base_struct base;
  user_struct_t user;
  struct array_container arrays;
  struct linked_node list;
  struct string_container strings;
  struct scalar_container scalars;
  struct callback_container callbacks;
  struct complex_root complex;
  struct tree_node tree;
  class lang_class lang_obj;
  
  /* Union type */
  union tagged_union current_union;
  
  /* Direct pointers */
  struct base_struct *ptr1;      /* TYPE_POINTER */
  user_struct_t *ptr2;           /* TYPE_POINTER */
  struct array_container *ptr3;   /* TYPE_POINTER */
  
  /* Arrays of different types */
  struct base_struct *ptr_array[5];          /* TYPE_ARRAY */
  user_struct_t *user_array[3];              /* TYPE_ARRAY */
  union tagged_union union_array[2];         /* TYPE_ARRAY */
  
  /* String array */
  const char * GTY((string)) string_array[4];
  
  /* Callback */
  callback_func handlers[2];
};

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) undefined_struct;

/* Pointer to undefined type */
struct GTY(()) has_undefined {
  struct undefined_struct *undefined_ptr;  /* Will be TYPE_UNDEFINED */
  int defined_field;
};

#endif /* TEST_GTY_INPUT_H */
