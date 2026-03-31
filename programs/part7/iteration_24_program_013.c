/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

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
  long field2;
  struct my_struct* GTY((skip)) next;  /* Using skip option */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* GTY((tag("0"))) p;  /* Using tag option */
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with desc tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) str;
    int GTY((tag("1"))) val;
  } u;
  struct lang_struct* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct lang_struct* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) prev;
};

/* Now define the previously opaque struct to complete TYPE_UNDEFINED -> TYPE_STRUCT */
struct GTY(()) opaque_struct {
  int id;
  struct opaque_struct* GTY((skip)) link;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* Contains various type kinds */
  struct my_struct GTY((skip)) plain_struct;      /* TYPE_STRUCT */
  union my_union GTY((skip)) plain_union;         /* TYPE_UNION */
  struct user_struct GTY((skip)) user;            /* TYPE_USER_STRUCT */
  struct lang_struct* GTY((skip)) lang;           /* TYPE_POINTER to TYPE_LANG_STRUCT */
  int_array numbers;                              /* TYPE_ARRAY */
  struct opaque_struct* GTY((skip)) opaque;       /* TYPE_POINTER */
  callback_fn handler;                            /* TYPE_CALLBACK */
  const char* GTY((skip)) name;                   /* TYPE_STRING */
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct container global_container;
extern GTY(()) struct opaque_struct* global_opaque_ptr;

/* Array of pointers with length option */
struct GTY(()) ptr_container {
  int count;
  struct my_struct* GTY((length("%h.count"))) items[10];
};

/* Union containing different pointer types */
union GTY(()) mixed_ptrs {
  struct my_struct* GTY((tag("0"))) s_ptr;
  union my_union* GTY((tag("1"))) u_ptr;
  struct lang_struct* GTY((tag("2"))) l_ptr;
  callback_fn GTY((tag("3"))) cb_ptr;
};

#endif /* TEST_COVERAGE_H */
