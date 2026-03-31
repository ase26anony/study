/* Test header for gengtype coverage of type state writing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_SCALAR: Basic scalar types */
typedef enum {
  CODE_NONE,
  CODE_STRUCT,
  CODE_UNION
} gty_code GTY((scalar));

/* TYPE_STRUCT: Basic structure with pointers */
struct GTY((chain_next ("%h.next"))) base_struct {
  struct base_struct * GTY((skip)) next;
  int data;
  gty_code code;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct *base_ptr;
  const char * GTY((tag ("0"))) name;
} user_struct_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.utype"))) my_union {
  struct base_struct * GTY((tag ("0"))) ptr_base;
  user_struct_t * GTY((tag ("1"))) ptr_user;
  int utype;
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array */
  struct base_struct * GTY((length ("10"))) fixed_array[10];
  
  /* Variable-length array */
  user_struct_t ** GTY((length ("%h.var_len"))) var_array;
  int var_len;
  
  /* Nested array */
  union my_union nested_array[5];
};

/* TYPE_POINTER: Typedef for pointer type */
typedef struct base_struct * GTY(()) base_ptr_t;

/* TYPE_STRING: String type */
struct GTY(()) string_container {
  const char * GTY((string)) str_field;
  char * GTY((string)) mutable_str;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) gty_callback)(struct base_struct *data);

struct GTY(()) callback_container {
  gty_callback callback;
  void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY(()) lang_class {
public:
  struct base_struct *member;
  user_struct_t *user;
  int lang_data;
  
  virtual ~lang_class() {}
};

#ifdef __cplusplus
}
#endif

/* Complex nested structure to ensure all types are connected */
struct GTY(()) root_container {
  /* TYPE_STRUCT */
  struct base_struct root_base;
  
  /* TYPE_USER_STRUCT */
  user_struct_t *user;
  
  /* TYPE_UNION */
  union myunion {
    struct array_container * GTY((tag ("0"))) array_ptr;
    struct callback_container * GTY((tag ("1"))) callback_ptr;
    int which;
  } GTY((desc ("%0.which"))) data_union;
  
  /* TYPE_ARRAY of unions */
  union my_union union_array[3];
  
  /* TYPE_POINTER typedef */
  base_ptr_t base_pointer;
  
  /* TYPE_STRING */
  struct string_container strings;
  
  /* TYPE_CALLBACK */
  gty_callback handlers[2];
  
  /* TYPE_LANG_STRUCT */
  class lang_class *lang_obj;
  
  /* TYPE_SCALAR */
  gty_code root_code;
  int scalar_field;
};

/* TYPE_UNDEFINED: Forward declaration */
struct GTY(()) undefined_struct;

/* Self-referential structure that uses undefined type */
struct GTY(()) defined_struct {
  struct undefined_struct *undef_ptr;
  struct defined_struct *self_ptr;
};

/* Now define the undefined struct */
struct GTY(()) undefined_struct {
  struct defined_struct *def_ptr;
  int value;
};

/* Chain of structures for traversal */
struct GTY((chain_next ("%h.next"))) chain_elem {
  struct chain_elem *next;
  struct chain_elem * GTY((skip)) prev;
  union my_union data;
  int id;
};

#endif /* TEST_GTY_INPUT_H */
