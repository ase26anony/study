/* Test header to cover all gengtype-state.cc switch cases */

/* TYPE_UNDEFINED - forward declaration */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR */
extern GTY(()) int global_scalar;

/* TYPE_STRING */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_STRUCT - plain struct */
struct GTY(()) my_struct {
  int field1;
  void* GTY((skip)) field2;  /* skip option */
  struct opaque_struct* GTY((tag("0"))) opaque_ptr;
};

/* TYPE_USER_STRUCT - with user option */
struct GTY((user)) user_struct {
  void* data;
  int GTY((skip)) skipped_field;
};

/* TYPE_UNION */
union GTY(()) my_union {
  int i;
  void* p;
  struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_POINTER */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT - with desc tag for language-specific */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union {
    int ival;
    const char* sval;
    struct my_struct* GTY((tag("1"))) mval;
  } GTY((tag("0"))) u;
  struct lang_struct* GTY((chain_next)) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
  /* TYPE_STRUCT member */
  struct my_struct nested_struct;
  
  /* TYPE_UNION member */
  union my_struct* GTY((tag("0"))) nested_union_ptr;
  
  /* TYPE_ARRAY member */
  int GTY(()) nested_array[20];
  
  /* TYPE_POINTER to TYPE_ARRAY */
  int_array* GTY(()) array_ptr;
  
  /* TYPE_POINTER to TYPE_LANG_STRUCT */
  struct lang_struct* GTY(()) lang_struct_ptr;
  
  /* TYPE_CALLBACK member */
  callback_fn handler;
  
  /* Chain for linked list */
  struct container* GTY((chain_next)) next;
  struct container* GTY((chain_prev)) prev;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container global_container;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct* global_lang_struct_list;

/* Now define the TYPE_UNDEFINED type */
struct GTY(()) opaque_struct {
  int defined_now;
  struct container* GTY(()) cont_ptr;
};

/* Array of pointers with length option */
struct GTY(()) ptr_array_container {
  int count;
  struct my_struct* GTY((length("count"))) items[1];
};

/* Union with nested struct */
union GTY(()) complex_union {
  struct {
    int type;
    void* GTY((tag("type"))) data;
  } tagged;
  long long as_int;
};

/* String array */
typedef const char* GTY(()) string_array[5];
extern GTY(()) string_array global_strings;
