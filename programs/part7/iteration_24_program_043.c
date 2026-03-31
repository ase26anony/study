/* test-coverage.h - Comprehensive GTY type definitions for gengtype-state.cc coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
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
  long field2;
  struct opaque_struct* GTY(()) opaque_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((skip)) p;
  double d;
  struct my_struct* GTY(()) s;
};

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_LANG_STRUCT: Language-specific structure with tagging */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    struct my_struct* GTY((tag("0"))) s;
    union my_union* GTY((tag("1"))) u;
    int_array* GTY((tag("2"))) a;
  } GTY((tag("code"))) u;
  struct lang_struct* GTY((chain_next)) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* TYPE_STRUCT nested field */
  struct my_struct nested_struct;
  
  /* TYPE_UNION nested field */
  union my_union nested_union;
  
  /* TYPE_POINTER nested field */
  my_ptr nested_pointer;
  
  /* TYPE_ARRAY nested field */
  int_array nested_array;
  
  /* TYPE_LANG_STRUCT nested field */
  struct lang_struct* GTY(()) lang_struct_ptr;
  
  /* TYPE_USER_STRUCT nested field */
  struct user_struct* GTY(()) user_struct_ptr;
  
  /* TYPE_CALLBACK nested field */
  callback_fn nested_callback;
  
  /* Chain pointers for linked list */
  struct container_struct* GTY((chain_next)) next;
  struct container_struct* GTY((chain_prev)) prev;
};

/* Variable-length array with length field */
struct GTY(()) varray_struct {
  int count;
  struct my_struct* GTY((length("count"))) items[1];
};

/* Another complex type with skip option */
struct GTY(()) skip_struct {
  void* GTY((skip)) skipped_ptr;
  int important_data;
  struct skip_struct* GTY(()) next_important;
};

/* Now define the previously opaque TYPE_UNDEFINED type */
struct GTY(()) opaque_struct {
  int defined_now;
  struct container_struct* GTY(()) container;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container_struct* GTY(()) global_container_list;
extern GTY(()) struct lang_struct* GTY(()) global_lang_struct;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct opaque_struct global_opaque_struct;
extern GTY(()) struct varray_struct* GTY(()) global_varray;
extern GTY(()) struct skip_struct* GTY(()) global_skip_list;

/* Array of pointers */
extern GTY(()) struct my_struct* GTY(()) global_struct_array[20];

/* Union containing various types */
union GTY(()) complex_union {
  struct my_struct* GTY(()) s;
  union my_union* GTY(()) u;
  struct lang_struct* GTY(()) l;
  callback_fn f;
  int_array* a;
};

/* Struct with nested anonymous union */
struct GTY(()) anon_union_struct {
  int type;
  union {
    int i;
    double d;
    struct my_struct* GTY(()) s;
  } GTY((tag("type"))) data;
};

#endif /* TEST_COVERAGE_H */
