/* Comprehensive GTY test coverage header for gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
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
  void* GTY((skip)) field2;  /* Using skip option */
};

/* TYPE_USER_STRUCT: User-defined marking */
struct GTY((user)) user_struct {
  void* data;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  struct my_struct* GTY((tag("0"))) s;  /* Using tag option */
  double d;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct GTY(()) struct_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes { TEST_NODE_CODE = 1 };
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
  } u;
};

/* Complex nesting to ensure deep traversal */
struct GTY(()) container {
  /* TYPE_STRUCT member */
  struct my_struct embedded_struct;
  
  /* TYPE_UNION member */
  union my_union embedded_union;
  
  /* TYPE_POINTER member with chain_next option */
  struct container* GTY((chain_next("%0.next"))) next;
  
  /* TYPE_ARRAY member */
  int_array number_array;
  
  /* TYPE_ARRAY of structs */
  struct_array struct_members;
  
  /* TYPE_POINTER to lang_struct */
  struct lang_struct* GTY((tag("0"))) lang_ptr;
  
  /* TYPE_STRING member */
  const char* GTY((length("%0.strlen"))) str;
  size_t strlen;
  
  /* TYPE_CALLBACK member */
  callback_fn handler;
  
  /* TYPE_SCALAR member */
  int counter;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct container global_container;
extern GTY(()) union my_union global_union;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct* global_lang_struct_ptr;

/* TYPE_UNDEFINED: Now define the previously forward-declared struct */
struct GTY(()) opaque_struct {
  struct container* ptr;
  int value;
};

/* Variable using the now-defined opaque struct */
extern GTY(()) struct opaque_struct global_opaque;
