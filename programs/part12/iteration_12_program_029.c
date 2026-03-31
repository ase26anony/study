#ifndef TEST_GTY_INPUT_H
#define TEST_GTY_INPUT_H

/* TYPE_STRUCT - Basic structure with GC marking */
struct GTY(()) base_struct {
  int id;
  const char *name;
};

/* TYPE_USER_STRUCT - Typedef'd structure */
typedef struct GTY(()) user_struct {
  struct base_struct *base;  /* TYPE_POINTER */
  int value;
} user_struct_t;

/* TYPE_UNION - Discriminated union with pointers */
union GTY((desc ("%0.type"))) data_union {
  struct base_struct * GTY((tag ("0"))) ptr_base;  /* TYPE_POINTER */
  user_struct_t * GTY((tag ("1"))) ptr_user;       /* TYPE_POINTER */
  int type;
};

/* TYPE_ARRAY - Structure with array of pointers */
struct GTY(()) array_container {
  int count;
  struct base_struct * GTY((length ("%h.count"))) items[1];  /* TYPE_ARRAY */
};

/* Variable length array */
struct GTY(()) varray_container {
  int length;
  user_struct_t * GTY((variable_length)) varray[1];  /* TYPE_ARRAY */
};

/* TYPE_POINTER - Typedef for pointer type */
typedef struct base_struct * GTY(()) base_ptr_t;

/* Linked list structure using chain_next */
struct GTY((chain_next ("%h.next"))) linked_list {
  struct linked_list *next;
  union data_union data;  /* TYPE_UNION */
  int list_id;
};

/* Nested structure with multiple pointer types */
struct GTY(()) complex_struct {
  struct linked_list *list_head;          /* TYPE_POINTER */
  struct array_container *arrays[5];      /* TYPE_ARRAY of TYPE_POINTER */
  union data_union current;               /* TYPE_UNION */
  base_ptr_t optional;                    /* TYPE_POINTER */
  const char * GTY((string)) description; /* TYPE_STRING */
  enum { RED, GREEN, BLUE } color;        /* TYPE_SCALAR */
};

/* TYPE_CALLBACK - Function pointer type */
typedef void (* GTY((callback)) callback_func)(struct complex_struct *data);

/* Structure with callback */
struct GTY(()) callback_container {
  callback_func handler;  /* TYPE_CALLBACK */
  struct complex_struct *target;  /* TYPE_POINTER */
};

/* TYPE_LANG_STRUCT - C++ class (must be in extern "C" block for gengtype) */
#ifdef __cplusplus
extern "C" {
#endif

class GTY(()) lang_class {
public:
  struct complex_struct *member;  /* TYPE_POINTER */
  int lang_id;
  
  /* Virtual method to ensure it's treated as C++ */
  virtual ~lang_class() {}
};

#ifdef __cplusplus
}
#endif

/* Root structure that references everything */
struct GTY(()) root_container {
  struct base_struct base_instance;
  user_struct_t user_instance;
  struct linked_list *current_list;      /* TYPE_POINTER */
  struct array_container *active_array;  /* TYPE_POINTER */
  struct callback_container callbacks[3]; /* TYPE_ARRAY of TYPE_STRUCT */
  class lang_class *lang_obj;            /* TYPE_POINTER to TYPE_LANG_STRUCT */
  
  /* Undefined type forward declaration */
  struct undefined_struct *undef_ptr;    /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Forward declaration for undefined type */
struct undefined_struct;

#endif /* TEST_GTY_INPUT_H */
