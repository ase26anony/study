/* Test header to cover all gengtype-state.cc switch cases */

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR - fundamental scalar type */
extern GTY(()) int global_scalar;

/* TYPE_STRING - string type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK - function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT - plain C struct */
struct GTY(()) my_struct {
  int field1;
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Forward reference */
  callback_fn GTY((skip)) handler;
};

/* TYPE_USER_STRUCT - struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int GTY((skip)) tag;
};

/* TYPE_UNION */
union GTY(()) my_union {
  int i;
  void* p;
  struct my_struct* GTY((skip)) s;
};

/* TYPE_POINTER - pointer typedef */
typedef struct my_struct* GTY(()) my_struct_ptr;
typedef union my_union* GTY(()) my_union_ptr;

/* TYPE_ARRAY - fixed-size array */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT - language-specific structure */
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
      char* GTY((length("strlen(%h.str)+1"))) str;
    } type2;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Chain of structures */
  struct complex_nested* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct complex_nested* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) prev;
  
  /* Array of pointers */
  struct my_struct* GTY(()) ptr_array[3];
  
  /* Union field */
  union my_union data_union;
  
  /* String with length */
  char* GTY((length("strlen(%h.dynamic_str)+1"))) dynamic_str;
  
  /* Scalar */
  int count;
  
  /* Callback */
  callback_fn GTY((skip)) notify;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested* global_nested_ptr;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct global_lang_struct;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct* GTY((skip)) link;
};

/* Array of unions */
union GTY(()) union_array[4];

/* Struct with array length depending on another field */
struct GTY(()) variable_length {
  int count;
  int GTY((length("%h.count"))) data[];
};
