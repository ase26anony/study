/* Test header to cover all gengtype-state.cc switch cases */

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR */
extern GTY(()) int global_scalar;

/* TYPE_STRING */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT - basic struct */
struct GTY(()) my_struct {
  int field1;
  long field2;
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* forward reference */
};

/* TYPE_USER_STRUCT - with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int tag;
};

/* TYPE_UNION */
union GTY(()) my_union {
  int i;
  float f;
  double d;
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_POINTER */
typedef struct my_struct* GTY(()) my_struct_ptr;
typedef union my_union* GTY(()) my_union_ptr;

/* TYPE_LANG_STRUCT - mimicking GCC tree structure */
enum test_node_code {
  TEST_NODE_CODE1,
  TEST_NODE_CODE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_code code;
  union GTY((desc ("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
    const char* GTY((tag("2"))) name;
  } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_UNION member */
  union my_union nested_union;
  
  /* TYPE_POINTER members */
  my_struct_ptr struct_ptr;
  my_union_ptr union_ptr;
  
  /* TYPE_ARRAY members */
  int_array ints;
  struct_ptr_array struct_ptrs;
  
  /* TYPE_LANG_STRUCT member */
  struct lang_struct* GTY((tag("0"))) lang_node;
  
  /* TYPE_STRING member */
  const char* GTY((skip)) description;
  
  /* TYPE_CALLBACK member */
  callback_fn handler;
  
  /* Chain pointers for linked list */
  struct container_struct* GTY((skip)) next;
  struct container_struct* GTY((skip)) prev;
};

/* TYPE_UNDEFINED - now define the forward-declared struct */
struct GTY(()) opaque_struct {
  int magic;
  struct container_struct* GTY((skip)) container;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container_struct* GTY((chain_next("next"), chain_prev("prev"))) global_container_list;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct opaque_struct global_opaque;

/* Array of pointers with length specifier */
struct GTY(()) ptr_array_container {
  int count;
  struct my_struct* GTY((length("%h.count"))) items[1];
};

/* Union with desc/tag for conditional fields */
union GTY((desc("TAG_VALUE"))) tagged_union {
  int GTY((tag("0"))) as_int;
  float GTY((tag("1"))) as_float;
  struct my_struct* GTY((tag("2"))) as_struct;
};

/* Struct with nested anonymous union */
struct GTY(()) with_anon_union {
  int type;
  union {
    int i;
    float f;
    void* p;
  } GTY((skip)) data;
};

/* For testing skip option */
struct GTY(()) skip_test {
  int GTY((skip)) skipped_field;
  int kept_field;
  void* GTY((skip)) skipped_ptr;
};

/* Testing callback in struct */
struct GTY(()) with_callback {
  const char* name;
  callback_fn GTY((skip)) callback;
  void (* GTY((skip)) another_callback)(int, char*);
};
