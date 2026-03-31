/* test-coverage.h - Comprehensive GTY test for gengtype-state.cc coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
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
};

/* TYPE_USER_STRUCT: User-defined marking routines */
struct GTY((user)) user_struct {
  void* data;
  struct user_struct* GTY((skip)) next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    int ival;
    double dval;
    struct lang_struct* GTY((tag("0"))) child;
  } u;
  struct lang_struct* GTY((chain_next)) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* Contains various type kinds */
  struct my_struct GTY((tag("STRUCT"))) nested_struct;
  union my_union GTY((tag("UNION"))) nested_union;
  struct my_struct* GTY((tag("POINTER"))) struct_ptr;
  int GTY((tag("SCALAR"))) count;
  const char* GTY((tag("STRING"))) name;
  callback_fn GTY((tag("CALLBACK"))) handler;
  int_array GTY((tag("ARRAY"))) numbers;
  struct lang_struct* GTY((tag("LANG_STRUCT"))) lang_node;
  struct opaque_struct* GTY((tag("OPAQUE"))) opaque_ptr;
  
  /* Chain pointers for linked list */
  struct container* GTY((chain_next)) next;
  struct container* GTY((chain_prev)) prev;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct container global_container;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) int_array global_int_array;

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
  int id;
  struct container* GTY((skip)) ref;
};

/* Array of pointers with length field */
struct GTY(()) variable_array {
  int GTY((length("%0.count"))) count;
  struct my_struct* GTY((ptr)) GTY((length("%0.count"))) items;
};

/* Union with tag for discriminant */
union GTY((tag("TYPE"))) tagged_union {
  int type;
  struct GTY((tag("1"))) {
    int x;
    int y;
  } point;
  struct GTY((tag("2"))) {
    char* GTY((length("%0.len"))) str;
    int len;
  } string;
};

#endif /* TEST_COVERAGE_H */
