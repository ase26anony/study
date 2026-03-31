/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates an undefined type initially */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  void* GTY((skip)) field2;  /* Using skip option */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  struct user_struct* GTY((skip)) next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* p;
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with desc tag */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_codes code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
    const char* GTY((tag("2"))) name;
  } u;
};

/* Now define the previously opaque struct to complete TYPE_UNDEFINED -> TYPE_STRUCT */
struct GTY(()) opaque_struct {
  int id;
  struct opaque_struct* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct opaque_struct* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) prev;
  union my_union data;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct plain_struct;          /* TYPE_STRUCT */
  struct user_struct* user_struct_ptr;    /* TYPE_POINTER to TYPE_USER_STRUCT */
  union my_union data_union;              /* TYPE_UNION */
  int_array number_array;                 /* TYPE_ARRAY */
  struct lang_struct* lang_struct_ptr;    /* TYPE_POINTER to TYPE_LANG_STRUCT */
  callback_fn callback_field;             /* TYPE_CALLBACK */
  const char* string_field;               /* TYPE_STRING */
  struct complex_nested* GTY((skip)) self_ptr; /* TYPE_POINTER with skip */
  
  /* Variable length array using length option */
  struct my_struct** GTY((length("%h.var_len"))) var_array;
  int var_len;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) struct user_struct* global_user_struct_list;
extern GTY(()) union my_union global_union;
extern GTY(()) struct lang_struct* global_lang_tree;
extern GTY(()) struct complex_nested global_complex;
extern GTY(()) struct opaque_struct* global_opaque_list;

/* Array of pointers with chain_next for linked list simulation */
struct GTY(()) list_node {
  int value;
  struct list_node* GTY((chain_next("%0.next"))) next;
};

/* Another complex type mixing arrays and pointers */
struct GTY(()) mixed_container {
  /* Array of structs */
  struct my_struct struct_array[3];
  
  /* Pointer to array */
  int (* GTY(()) array_ptr)[10];
  
  /* Nested array of pointers */
  struct list_node* GTY(()) node_array[5];
  
  /* Union containing different pointer types */
  union {
    struct my_struct* s_ptr;
    struct list_node* l_ptr;
    void* v_ptr;
  } GTY((tag("0"))) ptr_union;
};

/* Additional callback type variations */
typedef int (* GTY(()) int_callback)(int, void*);
typedef struct my_struct* (* GTY(())) struct_factory)(void);

/* Global instances */
extern GTY(()) struct mixed_container global_container;
extern GTY(()) int_callback global_int_callback;
extern GTY(()) struct_factory global_factory;

#endif /* TEST_COVERAGE_H */
