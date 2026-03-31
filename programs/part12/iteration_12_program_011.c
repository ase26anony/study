/* Test file to cover all type classifications in gengtype-state.cc */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_A,
  CODE_B,
  CODE_C
} tree_code;

/* TYPE_STRING: String type */
typedef const char * GTY((string)) gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(void *data);

/* TYPE_STRUCT: Basic structure */
struct GTY(()) base_struct {
  int scalar_field;           /* TYPE_SCALAR */
  gty_string str_field;       /* TYPE_STRING */
  gty_callback callback_field; /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct * GTY((skip)) ptr_field;  /* TYPE_POINTER */
  int data;
} user_struct_t;

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  struct base_struct * GTY((length("5"))) fixed_array[5];
  
  /* Variable-length array */
  user_struct_t * GTY((variable_length)) var_array;
  int var_length;
  
  /* Nested array */
  int * GTY((length("%h.var_length"))) int_array;
};

/* TYPE_UNION: Discriminated union */
union GTY((desc("type_field"))) discriminated_union {
  struct base_struct * GTY((tag("0"))) ptr_base;    /* TYPE_POINTER */
  user_struct_t * GTY((tag("1"))) ptr_user;         /* TYPE_POINTER */
  struct array_container * GTY((tag("2"))) ptr_array; /* TYPE_POINTER */
  int type_field;                                   /* TYPE_SCALAR */
};

/* TYPE_POINTER: Special pointer typedef */
typedef struct base_struct * GTY((atomic)) base_ptr;

/* Linked list structure using chain_next */
struct GTY((chain_next("%h.next"))) linked_node {
  struct linked_node *next;
  union discriminated_union data;  /* TYPE_UNION */
  base_ptr optional_ptr;           /* TYPE_POINTER via typedef */
};

/* Complex nested structure to ensure traversal */
struct GTY(()) complex_root {
  /* Multiple pointer types */
  struct linked_node *list_head;           /* TYPE_POINTER */
  struct array_container *container;       /* TYPE_POINTER */
  
  /* Direct union */
  union discriminated_union current_union; /* TYPE_UNION */
  
  /* Array of unions */
  union discriminated_union GTY((length("3"))) union_array[3];
  
  /* Pointer to array */
  user_struct_t * GTY((length("%h.count"))) *ptr_to_array;
  int count;
  
  /* Nested structure */
  struct {
    struct base_struct *nested_ptr;        /* TYPE_POINTER */
    int nested_scalar;                     /* TYPE_SCALAR */
  } GTY((skip)) nested;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY(()) lang_class {
public:
  struct complex_root *root_ptr;           /* TYPE_POINTER */
  user_struct_t *data;                     /* TYPE_POINTER */
  int lang_specific_field;                 /* TYPE_SCALAR */
  
  /* Method pointer (not GTY, just for completeness) */
  void (*method)(void);
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) gty_root {
  struct linked_node *active_list;         /* TYPE_POINTER */
  class lang_class *lang_obj;              /* TYPE_POINTER to TYPE_LANG_STRUCT */
  struct complex_root *complex_data;       /* TYPE_POINTER */
  gty_string description;                  /* TYPE_STRING */
};

#endif /* TEST_GTY_INPUT_H */
