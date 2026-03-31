/* Test header to cover all gengtype-state.cc switch cases */

/* TYPE_UNDEFINED - forward declaration creates undefined type */
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
  long field2;
};

/* TYPE_USER_STRUCT - struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int counter;
};

/* TYPE_UNION - union type */
union GTY(()) my_union {
  int i;
  void* p;
  double d;
};

/* TYPE_POINTER - pointer type */
typedef struct my_struct * GTY(()) my_ptr;
typedef union my_union * GTY(()) union_ptr;

/* TYPE_ARRAY - fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct * GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT - language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2,
  TEST_NODE_TYPE3
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_codes code;
  union GTY((desc("1"))) {
    int ival;
    double dval;
    struct lang_struct * GTY((tag("0"))) child;
  } u;
  struct lang_struct * GTY((skip)) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_UNION member */
  union my_union nested_union;
  
  /* TYPE_POINTER member */
  struct my_struct * GTY((tag("0"))) nested_ptr;
  
  /* TYPE_ARRAY member */
  int GTY(()) nested_array[20];
  
  /* Chain pointers for GC */
  struct container_struct * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct container_struct * GTY((skip)) prev;
  
  /* String pointer */
  const char * GTY((length("strlen(%h.str_field)+1"))) str_field;
  
  /* Callback function pointer */
  callback_fn handler;
  
  /* Reference to user struct */
  struct user_struct * GTY((skip)) user_data;
  
  /* Language structure */
  struct lang_struct * GTY((tag("1"))) lang_node;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container_struct * GTY(()) global_container;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct * GTY(()) global_lang_struct;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct my_struct * GTY(()) ptr_to_struct;
};

/* Array of pointers with length field */
struct GTY(()) var_array_struct {
  int count;
  struct my_struct * GTY((length("%h.count"))) items[1];
};

/* Union with tag for discriminant */
union GTY((tag("CODE"))) tagged_union {
  int code;
  struct {
    int GTY((skip)) type;
    union tagged_union * GTY((tag("1"))) next;
  } s;
};

/* TYPE_CALLBACK in a struct context */
struct GTY(()) callback_container {
  callback_fn GTY(()) handlers[3];
  void (* GTY(()) single_handler)(int, char*);
};
