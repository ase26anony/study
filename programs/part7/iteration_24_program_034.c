/* Test header to cover all gengtype-state.cc switch cases */

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
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int user_tag;
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
typedef struct my_struct * GTY(()) ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
enum test_node_type { TEST_NODE_A, TEST_NODE_B, TEST_NODE_C };

struct GTY((desc("%0.test_node_type"))) lang_struct {
  enum test_node_type code;
  union GTY((desc("%1.code"))) {
    struct GTY((tag("0"))) {
      int int_val;
    } a;
    struct GTY((tag("1"))) {
      double double_val;
    } b;
    struct GTY((tag("2"))) {
      char* string_val;
    } c;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains a struct */
  struct my_struct embedded_struct;
  
  /* Contains a union */
  union my_union embedded_union;
  
  /* Contains a pointer */
  struct my_struct * GTY((skip)) pointer_field;
  
  /* Contains an array */
  int GTY(()) int_array_field[8];
  
  /* Contains a pointer to array */
  int (* GTY(()) array_ptr_field)[4];
  
  /* Chain pointers for linked list */
  struct complex_nested * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct complex_nested * GTY((skip)) prev;
  
  /* String field */
  const char* GTY(()) name;
  
  /* Callback field */
  callback_fn handler;
  
  /* User struct field */
  struct user_struct user_data;
};

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct complex_nested * GTY(()) nested;
};

/* Global variables to ensure they appear in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested * GTY(()) global_nested_ptr;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct opaque_struct global_opaque;

/* Array of pointers with length specifier */
struct GTY(()) ptr_container {
  int count;
  struct my_struct * GTY((length("%h.count"))) items[1];
};

/* Union containing different pointer types */
union GTY(()) mixed_union {
  struct my_struct * GTY((tag("0"))) s_ptr;
  union my_union * GTY((tag("1"))) u_ptr;
  int_array * GTY((tag("2"))) a_ptr;
};

/* Struct with conditional fields */
struct GTY(()) conditional_struct {
  int GTY(()) flag;
  union GTY((desc("%0.flag"))) {
    struct GTY((tag("0"))) {
      int x;
    } when_zero;
    struct GTY((tag("1"))) {
      double y;
      char* z;
    } when_one;
  } data;
};

#endif /* TEST_COVERAGE_H */
