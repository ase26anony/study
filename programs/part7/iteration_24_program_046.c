/* Test header for covering gengtype-state.cc switch cases */

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
  struct my_struct* GTY((skip)) next;  /* Using skip option */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* GTY((tag("0"))) p;  /* Using tag option */
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
    const char* GTY((tag("2"))) name;
  } u;
  struct lang_struct* GTY((chain_next("%0"))) next;
};

/* Now define the previously opaque struct to complete TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int id;
  struct my_struct* GTY(()) data;
  struct opaque_struct* GTY((chain_next("%0"))) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct base;              /* TYPE_STRUCT */
  union my_union variant;             /* TYPE_UNION */
  int_array numbers;                  /* TYPE_ARRAY */
  struct_ptr_array pointers;          /* TYPE_ARRAY of TYPE_POINTER */
  struct lang_struct* GTY(()) lang;   /* TYPE_POINTER to TYPE_LANG_STRUCT */
  callback_fn handler;                /* TYPE_CALLBACK */
  const char* GTY(()) description;    /* TYPE_STRING */
  
  /* Chain for linked list */
  struct complex_nested* GTY((chain_next("%0"))) next;
  struct complex_nested* GTY((chain_prev("%0"))) prev;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) struct complex_nested* global_complex_list;
extern GTY(()) struct opaque_struct* global_opaque_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) callback_fn global_callback_array[3];

/* Variable-length array using length option */
struct GTY(()) var_len_struct {
  int count;
  int GTY((length("%0.count"))) data[];
};

/* Another complex case: union containing struct with array */
union GTY(()) container_union {
  struct GTY(()) {
    int type;
    struct my_struct* GTY((skip)) items[4];
  } s;
  struct lang_struct* GTY(()) lang_item;
};

#endif /* TEST_COVERAGE_H */
