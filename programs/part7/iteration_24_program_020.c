/* Test header to cover all type kinds in gengtype-state.cc switch */

#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration creates undefined type reference */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  void * GTY((skip)) field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  struct user_struct *next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  double d;
};

/* TYPE_POINTER: Pointer type definition */
typedef struct my_struct * GTY(()) my_struct_ptr;
typedef union my_union * GTY(()) my_union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct GTY(()) struct_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((tag("0"))) {
    struct lang_struct * GTY((tag("1"))) child;
    int value;
  } u;
  struct lang_struct * GTY((chain_next)) next;
};

/* TYPE_SCALAR: Global scalar variable */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;
extern GTY(()) char* mutable_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
typedef int (* GTY(()) callback_with_arg)(int, void*);

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains a pointer (TYPE_POINTER) */
  struct my_struct * GTY(()) struct_ptr;
  
  /* Contains an array (TYPE_ARRAY) */
  int GTY(()) numbers[20];
  
  /* Contains a union (TYPE_UNION) */
  union my_union GTY(()) data;
  
  /* Contains another struct (TYPE_STRUCT) */
  struct my_struct GTY(()) embedded;
  
  /* Chain of structures */
  struct complex_nested * GTY((chain_next)) next;
  struct complex_nested * GTY((chain_prev)) prev;
  
  /* String field (TYPE_STRING) */
  const char* GTY(()) name;
  
  /* Callback field (TYPE_CALLBACK) */
  callback_fn GTY(()) handler;
  
  /* Pointer to lang_struct (TYPE_LANG_STRUCT) */
  struct lang_struct * GTY(()) lang_node;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested *global_nested_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) callback_fn global_callback;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct * GTY(()) link;
};

/* Array of pointers */
typedef struct opaque_struct * GTY(()) opaque_ptr_array[8];

/* Union containing various types */
union GTY(()) container_union {
  struct my_struct GTY(()) s;
  struct complex_nested * GTY(()) p;
  int GTY(()) arr[4];
  callback_fn GTY(()) cb;
};

/* Struct with length attribute for variable-sized array */
struct GTY(()) varray_struct {
  int count;
  int GTY((length("%0.count"))) items[1];
};

/* Skip option test */
struct GTY(()) skip_test {
  int used_field;
  void* GTY((skip)) skipped_field;
  struct skip_test * GTY((skip("custom_skip"))) custom_skip_field;
};

/* Nested anonymous union */
struct GTY(()) with_anon_union {
  int type;
  union {
    int int_val;
    double double_val;
    void* ptr_val;
  } GTY((tag("type"))) value;
};
