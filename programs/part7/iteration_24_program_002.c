/* Test header to cover gengtype-state.cc switch cases */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type as GC root */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  void* GTY((skip)) field2;  /* Use skip option */
  struct opaque_struct* GTY((tag("0"))) next;  /* Pointer with tag */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  /* User will provide marking routines elsewhere */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  double d;
  struct my_struct* GTY((tag("1"))) s;
  void* p;
};

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct* GTY(()) my_struct_ptr;
typedef union my_union* GTY(()) my_union_ptr;
typedef int_array* GTY(()) array_ptr;

/* TYPE_LANG_STRUCT: Language-specific structure with desc tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    int ival;
    double dval;
    struct lang_struct* GTY((tag("0"))) chain;
  } u;
  struct lang_struct* GTY((chain_next, chain_prev)) siblings;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* Contains one of each type */
  struct my_struct GTY(()) nested_struct;
  union my_union GTY(()) nested_union;
  struct lang_struct* GTY(()) lang_ptr;
  int_array GTY(()) numbers;
  struct_ptr_array GTY(()) pointers;
  callback_fn GTY(()) handler;
  const char* GTY(()) name;
  
  /* Chain for linked list */
  struct container* GTY((skip)) next;
  struct container* GTY((chain_next)) chain_next;
  struct container* GTY((chain_prev)) chain_prev;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) struct container* global_container_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct user_struct global_user_struct;

/* TYPE_UNDEFINED resolution: Now define the opaque struct */
struct GTY(()) opaque_struct {
  int id;
  struct container* GTY(()) data;
  struct opaque_struct* GTY((skip)) next;
};

/* Variable-length array using length option */
struct GTY(()) varray_struct {
  int count;
  int GTY((length("%0.count"))) values[1];
};

/* Nested anonymous union within struct */
struct GTY(()) anon_union_struct {
  int type;
  union {
    int num;
    char* GTY((tag("1"))) str;
    struct my_struct* GTY((tag("2"))) s;
  } GTY((desc("%0.type"))) data;
};

#endif /* TEST_COVERAGE_H */
