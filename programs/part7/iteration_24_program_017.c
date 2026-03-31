/* Test header to cover all gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type as GC root */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  double field2;
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

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct GTY(()) struct_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
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
      struct lang_struct* GTY((tag("0"))) next;
    } type2;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* TYPE_POINTER nested */
  struct my_struct* GTY(()) ptr_field;
  
  /* TYPE_ARRAY nested */
  int GTY(()) array_field[8];
  
  /* TYPE_UNION nested */
  union my_union GTY(()) union_field;
  
  /* TYPE_STRUCT nested */
  struct my_struct GTY(()) struct_field;
  
  /* TYPE_CALLBACK nested */
  callback_fn GTY(()) callback_field;
  
  /* Chain pointer for linked list */
  struct container* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct container* GTY((skip)) prev;
  
  /* Length field for variable array */
  int count;
  
  /* Variable length array */
  struct my_struct* GTY((length("%0.count"))) var_array;
};

/* TYPE_UNDEFINED: Now define the previously declared opaque type */
struct GTY(()) opaque_struct {
  int defined_field;
  struct container* GTY(()) cont;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct container global_container;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct opaque_struct global_opaque;
extern GTY(()) callback_fn global_callback;
extern GTY(()) int_array global_int_array;

/* Variable with callback type */
GTY(()) callback_fn active_callback = 0;

/* String array */
extern GTY(()) const char* GTY(()) string_array[];

/* Nested pointer chain */
struct GTY(()) pointer_chain {
  void* GTY(()) data;
  struct pointer_chain* GTY(()) next;
};

/* Root variable for pointer chain */
extern GTY(()) struct pointer_chain* chain_root;
