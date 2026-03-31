/* Test header for gengtype coverage of type state writing */

#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_STRUCT: Basic structure with GC marking */
struct GTY(()) base_struct {
  int id;
  const char* GTY((skip)) name;  /* TYPE_STRING via skip */
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct* GTY((tag("0"))) base;  /* TYPE_POINTER */
  int value;
} user_struct_t;

/* TYPE_UNION: Discriminated union */
union GTY((desc ("%0.type"))) data_union {
  struct base_struct* GTY((tag("1"))) ptr_base;  /* TYPE_POINTER */
  user_struct_t* GTY((tag("2"))) ptr_user;       /* TYPE_POINTER */
  int type;  /* TYPE_SCALAR discriminator */
};

/* TYPE_ARRAY: Structure with array fields */
struct GTY(()) array_container {
  /* Fixed-size array of pointers */
  user_struct_t* GTY((length ("10"))) fixed_array[10];
  
  /* Variable-length array */
  struct base_struct** GTY((variable_length)) var_array;
  unsigned int var_len;
  
  /* Nested array */
  union data_union GTY((length ("%h.var_len"))) union_array[1];
};

/* TYPE_POINTER: Special pointer typedef */
typedef user_struct_t* GTY((user)) user_ptr;

/* Linked list structure for chain_next/chain_prev */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
  struct linked_node* next;
  struct linked_node* prev;
  union data_union data;
  user_ptr alias;  /* TYPE_POINTER via typedef */
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY((callback)) event_callback)(struct base_struct* GTY((skip)), int);

/* Structure containing callback */
struct GTY(()) event_handler {
  event_callback callback;
  struct linked_node* GTY((skip)) context;
};

/* For TYPE_LANG_STRUCT - C++ class definition */
#ifdef __cplusplus
class GTY(()) cpp_class {
private:
  struct array_container* GTY((reorder ("cplusplus_reorder"))) container;
  user_struct_t* data;
  
public:
  cpp_class() : container(0), data(0) {}
  virtual ~cpp_class() {}
  
  void process() {
    if (container && data) {
      /* dummy processing */
    }
  }
};
#endif /* __cplusplus */

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_root {
  /* Direct types */
  struct base_struct base;
  user_struct_t user;
  union data_union data;
  
  /* Pointers */
  struct array_container* array_ptr;
  struct linked_node* list_head;
  user_ptr* ptr_array;  /* Array of pointers */
  
  /* Arrays */
  struct base_struct* GTY((length ("5"))) base_array[5];
  union data_union GTY((length ("3"))) union_members[3];
  
  /* Callback */
  event_callback notify;
  
  /* String */
  const char* GTY((string)) message;
  
  /* Scalar */
  enum { RED, GREEN, BLUE } color;
  unsigned int flags;
  
#ifdef __cplusplus
  /* C++ class member */
  class cpp_class* cpp_obj;
#endif
  
  /* Self-reference for cycles */
  struct complex_root* GTY((skip)) self;
};

/* TYPE_UNDEFINED trigger - forward declaration without definition */
struct GTY(()) forward_declared;

/* Another structure that references the forward-declared type */
struct GTY(()) uses_forward {
  struct forward_declared* GTY((skip)) forward_ptr;
  int valid;
};

#endif /* TEST_GTY_INPUT_H */
