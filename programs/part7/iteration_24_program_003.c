/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration creates an undefined type initially */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  void * GTY((skip)) field2;  /* Use skip option */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  struct user_struct * GTY((skip)) next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type definition */
typedef struct my_struct * GTY(()) my_ptr;
typedef union my_union * GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct * GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union tree_node * GTY((tag("0"))) u;
  struct lang_struct * GTY((chain_next)) chain;
};

/* TYPE_SCALAR: Fundamental scalar types as GC roots */
extern GTY(()) int global_scalar;
extern GTY(()) long global_long;
extern GTY(()) double global_double;

/* TYPE_STRING: String types */
extern GTY(()) const char* global_string;
extern GTY(()) char* mutable_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
typedef int (* GTY(()) int_callback)(int, char*);

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains a struct */
  struct my_struct embedded_struct;
  
  /* Contains a union */
  union my_union embedded_union;
  
  /* Contains a pointer */
  struct complex_nested * GTY((skip)) next;
  
  /* Contains an array */
  int GTY(()) numbers[20];
  
  /* Contains a pointer to array */
  int (* GTY(()) array_ptr)[10];
  
  /* Chain for linked list */
  struct complex_nested * GTY((chain_next)) chain_next;
  struct complex_nested * GTY((chain_prev)) chain_prev;
};

/* Now define the previously opaque struct to complete TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int id;
  struct my_struct * GTY(()) data;
  struct opaque_struct * GTY((skip)) next;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested *global_nested_ptr;
extern GTY(()) int_array global_int_array;
extern GTY(()) callback_fn global_callback;

/* Struct with length option for variable-sized array */
struct GTY(()) varray_struct {
  int count;
  int GTY((length("%0.count"))) items[1];
};

/* Union with tag for discriminated union */
union GTY((tag("CODE"))) tagged_union {
  int code;
  struct GTY((tag("1"))) {
    int x;
    int y;
  } point;
  struct GTY((tag("2"))) {
    char * GTY(()) name;
    int value;
  } named;
};

#endif /* TEST_COVERAGE_H */
