/* Test header to cover all gengtype-state.cc switch cases */
#ifndef GTYPE_COVERAGE_TEST_H
#define GTYPE_COVERAGE_TEST_H

/* TYPE_UNDEFINED: Forward declaration to create undefined type reference */
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
  long field2;
  struct opaque_struct* GTY((skip)) opaque_ptr;  /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int user_tag;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  float f;
  void* GTY((skip)) p;
  struct my_struct* GTY((skip)) s;
};

/* TYPE_POINTER: Pointer type definition */
typedef struct my_struct* GTY(()) my_struct_ptr;
typedef union my_union* GTY(()) my_union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
enum test_node_codes {
  TEST_NODE_TYPE1,
  TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((desc("%1.code"))) lang_union {
    struct lang_struct* GTY((tag("0"))) child;
    int GTY((tag("1"))) value;
    const char* GTY((tag("2"))) name;
  } u;
  struct lang_struct* GTY((chain_next("%0.u.child"))) next;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
  /* Nested struct */
  struct GTY(()) nested_struct {
    int id;
    char* GTY((length("strlen(%h.data) + 1"))) data;
  } nested;
  
  /* Pointer to union */
  union my_union* GTY((skip)) union_ptr;
  
  /* Array of structs */
  struct nested_struct GTY(()) struct_array[3];
  
  /* Pointer array */
  struct nested_struct* GTY(()) ptr_array[2];
  
  /* Chain of structures */
  struct container_struct* GTY((chain_next("%h.next"))) next;
  struct container_struct* GTY((chain_prev("%h.prev"))) prev;
  
  /* Callback field */
  callback_fn GTY((skip)) handler;
  
  /* String field */
  const char* GTY((skip)) description;
  
  /* Scalar field */
  unsigned int flags;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct container_struct global_container;
extern GTY(()) struct lang_struct* global_lang_struct_root;
extern GTY(()) int_array global_int_array;
extern GTY(()) my_struct_ptr global_struct_ptr;

/* TYPE_UNDEFINED: Now define the previously forward-declared struct */
struct opaque_struct {
  int defined_later;
  struct my_struct* GTY((skip)) link;
};

/* Template-like structure with conditional fields */
struct GTY(()) variant_struct {
  enum { VAR_INT, VAR_PTR, VAR_STR } type;
  union GTY((desc("%0.type"))) variant_data {
    int GTY((tag("VAR_INT"))) int_val;
    void* GTY((tag("VAR_PTR"))) ptr_val;
    char* GTY((tag("VAR_STR"))) str_val;
  } data;
};

#endif /* GTYPE_COVERAGE_TEST_H */
