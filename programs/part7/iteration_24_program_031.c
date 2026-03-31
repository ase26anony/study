/* Test header to cover all gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration creates undefined type reference */
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

/* TYPE_USER_STRUCT: User-defined marking routines */
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
typedef struct my_struct * GTY(()) ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("1"))) {
    struct my_struct * GTY((tag("0"))) s;
    union my_union * GTY((tag("1"))) u;
  } GTY((tag("code"))) data;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_UNION member */
  union my_union nested_union;
  
  /* TYPE_POINTER member */
  struct my_struct * GTY(()) struct_ptr;
  
  /* TYPE_ARRAY member */
  int GTY(()) int_arr[8];
  
  /* Chain of pointers for chain_next/prev testing */
  struct container_struct * GTY((chain_next("%h.next"))) next;
  struct container_struct * GTY((chain_prev("%h.prev"))) prev;
  
  /* String pointer */
  const char * GTY(()) name;
  
  /* Callback function pointer */
  callback_fn handler;
  
  /* Pointer to lang_struct */
  struct lang_struct * GTY(()) lang_ptr;
  
  /* Variable length array with length specifier */
  struct my_struct * GTY((length("%h.var_len"))) var_array;
  size_t var_len;
};

/* Variable length array with nested structure */
struct GTY(()) var_len_struct {
  int count;
  struct my_struct GTY((length("%h.count"))) items[1];
};

/* Skip option testing */
struct GTY(()) skip_test {
  void* GTY((skip)) skipped_ptr;
  int GTY(()) kept_field;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct container_struct global_container;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) int_array global_int_array;
extern GTY(()) union my_union global_union;
extern GTY(()) struct user_struct global_user_struct;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct container_struct * GTY(()) link;
};

/* Array of pointers with chain */
struct GTY(()) ptr_chain {
  struct container_struct * GTY(()) items[4];
  struct ptr_chain * GTY((chain_next("%h.next_chain"))) next_chain;
};

/* Test structure with conditional fields */
struct GTY(()) conditional_struct {
  int type;
  union GTY((desc("%0.type"))) {
    int GTY((tag("0"))) as_int;
    double GTY((tag("1"))) as_double;
    struct my_struct * GTY((tag("2"))) as_struct;
  } GTY((tag("type"))) value;
};

/* String array */
typedef const char * GTY(()) string_array[3];

/* Multi-dimensional array */
typedef int GTY(()) matrix[4][4];

/* Complete the opaque type definition cycle */
struct forward_ref_test {
  struct opaque_struct * GTY(()) opaque_ptr;
  struct forward_ref_test * GTY(()) self_ref;
};
