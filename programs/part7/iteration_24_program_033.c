/* Test header to cover all gengtype-state.cc type kind cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
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
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int counter;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct * GTY(()) my_ptr;
typedef union my_union * GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct * GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct GTY((tag("0"))) {
      int int_val;
    } type1;
    struct GTY((tag("1"))) {
      double double_val;
      struct lang_struct * GTY((tag("0"))) next;
    } type2;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains a pointer (TYPE_POINTER) */
  struct my_struct * GTY(()) ptr_field;
  
  /* Contains an array (TYPE_ARRAY) */
  int GTY(()) array_field[8];
  
  /* Contains a union (TYPE_UNION) */
  union my_union GTY(()) union_field;
  
  /* Contains a callback (TYPE_CALLBACK) */
  callback_fn GTY(()) callback_field;
  
  /* Chain pointer for linked list */
  struct complex_nested * GTY((skip)) next;
  struct complex_nested * GTY((skip)) prev;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct lang_struct global_lang_struct_var;
extern GTY(()) struct complex_nested *global_nested_list;
extern GTY(()) int_array global_int_array;

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
  int defined_field;
  struct my_struct * GTY(()) related;
};

/* Array of pointers with length field */
struct GTY(()) variable_length_struct {
  int count;
  struct my_struct * GTY((length("%h.count"))) items[1];
};

/* Struct with nested struct */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_field;
    struct outer_struct * GTY((skip)) parent;
  } inner;
  int outer_field;
};

#endif /* TEST_COVERAGE_H */
