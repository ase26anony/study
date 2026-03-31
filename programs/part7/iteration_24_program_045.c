/* Test header for covering gengtype-state.cc switch cases */

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
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Uses undefined type */
};

/* TYPE_USER_STRUCT - struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int length;
};

/* TYPE_UNION - union type */
union GTY(()) my_union {
  int i;
  void* GTY((tag("0"))) p;
  double d;
};

/* TYPE_POINTER - pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY - fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT - language-specific structure */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("%0.code"))) lang_struct {
  enum test_node_codes code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("TEST_NODE_TYPE1"))) child;
    int GTY((tag("TEST_NODE_TYPE2"))) value;
  } u;
  struct lang_struct* GTY((chain_next("%0"))) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_UNION member */
  union my_union nested_union;
  
  /* TYPE_POINTER member */
  my_ptr struct_pointer;
  
  /* TYPE_ARRAY member */
  int_array number_array;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_struct* GTY((skip)) lang_ptr;
  
  /* TYPE_CALLBACK member */
  callback_fn handler;
  
  /* TYPE_USER_STRUCT member */
  struct user_struct user_data;
  
  /* Chain for linked list */
  struct container_struct* GTY((chain_next("%h.next"))) next;
  struct container_struct* GTY((chain_prev("%h.prev"))) prev;
};

/* Variable-length array with length field */
struct GTY(()) varray_struct {
  int count;
  struct my_struct* GTY((length("%h.count"))) items;
};

/* Another complex type with skip option */
struct GTY(()) skip_test {
  int id;
  void* GTY((skip)) skipped_ptr;
  const char* GTY((skip)) skipped_string;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct container_struct* global_container;
extern GTY(()) struct varray_struct global_varray;
extern GTY(()) struct skip_test global_skip_test;
extern GTY(()) union my_union global_union_var;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_now;
  struct container_struct* GTY((skip)) container_ref;
};

/* Array of pointers with different types */
typedef struct GTY(()) {
  struct my_struct* GTY(()) ptr1;
  union my_union* GTY(()) ptr2;
  struct lang_struct* GTY(()) ptr3;
} multi_ptr_container;

extern GTY(()) multi_ptr_container global_multi_ptrs;

/* String array */
typedef const char* GTY(()) string_array[3];
extern GTY(()) string_array global_strings;
