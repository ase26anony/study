/* Test header to cover all gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
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
  struct opaque_struct* GTY(()) ptr_field;  /* Pointer to undefined type */
  callback_fn GTY(()) cb_field;             /* Callback field */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;  /* skip option indicates user handles marking */
  int length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  struct my_struct* GTY(()) s;
  const char* GTY(()) str;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) {
    struct lang_struct* GTY((tag("0"))) child;
    const char* GTY((tag("1"))) name;
    int GTY((tag("2"))) value;
  } u;
  struct lang_struct* GTY((chain_next("%0"))) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* TYPE_STRUCT nested */
  struct my_struct GTY(()) nested_struct;
  
  /* TYPE_UNION nested */
  union my_union GTY(()) nested_union;
  
  /* TYPE_POINTER nested */
  my_ptr GTY(()) nested_ptr;
  
  /* TYPE_ARRAY nested */
  int_array GTY(()) nested_array;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_struct* GTY(()) lang_ptr;
  
  /* TYPE_USER_STRUCT */
  struct user_struct GTY(()) user_data;
  
  /* TYPE_STRING */
  const char* GTY(()) description;
  
  /* TYPE_CALLBACK */
  callback_fn GTY(()) handler;
  
  /* Chain for linked list */
  struct container* GTY((skip)) next;
  struct container* GTY((skip)) prev;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) my_ptr global_my_ptr;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct* global_lang_struct;
extern GTY(()) struct container* global_container_list;

/* Now define the previously undefined struct */
struct opaque_struct {
  int defined_field;
  struct container* GTY(()) container_ref;
};

/* Array of pointers with length field */
struct GTY(()) variable_array_container {
  int count;
  struct my_struct* GTY((length("%0.count"))) items[1];
};

/* Union with tag */
union GTY((tag("TYPE"))) tagged_union {
  int GTY((tag("0"))) type_int;
  float GTY((tag("1"))) type_float;
  struct my_struct* GTY((tag("2"))) type_struct;
};

/* Struct with callback array */
struct GTY(()) callback_container {
  int count;
  callback_fn GTY((length("%0.count"))) callbacks[3];
};

/* String array */
typedef const char* GTY(()) string_array[5];
extern GTY(()) string_array global_strings;
