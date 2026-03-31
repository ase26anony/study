/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_SCALAR - fundamental type */
typedef int my_scalar;

/* TYPE_STRING - string type */
typedef const char *my_string;

/* TYPE_POINTER - pointer type */
struct pointer_container {
  int *int_ptr;
  struct opaque_struct *struct_ptr;
};

/* TYPE_ARRAY - array type */
struct array_container {
  int int_array[10];
  char *string_array[5];
};

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int id;
  char *name;
  struct my_struct *next;
};

/* TYPE_UNION - union type */
union my_union {
  int int_val;
  float float_val;
  char *string_val;
  void *ptr_val;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
  void *user_handle;
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, const char*);

struct callback_container {
  callback_func handler;
  void (*simple_callback)(void);
};

/* TYPE_LANG_STRUCT - simulate GCC language-specific type */
#ifdef IN_GCC
/* This would normally be tree_node or similar */
struct GTY((tag("TS_BASE"))) lang_struct {
  enum tree_code code;
  union lang_struct_u {
    struct lang_struct_base base;
    struct lang_struct_decl decl;
  } GTY((desc("TREE_CODE((%h.code))"))) u;
};
#else
/* Simplified version for testing */
struct GTY((desc("0"))) lang_struct {
  int lang_specific;
  void *lang_data;
};
#endif

/* Complex nested structure to ensure all types are processed */
struct GTY(()) master_container {
  /* TYPE_STRUCT */
  struct my_struct nested_struct;
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* TYPE_POINTER */
  struct pointer_container *ptr_container;
  
  /* TYPE_ARRAY */
  struct array_container array_container;
  
  /* TYPE_SCALAR */
  my_scalar scalar_value;
  
  /* TYPE_STRING */
  my_string description;
  
  /* TYPE_CALLBACK */
  callback_func notify;
  
  /* TYPE_USER_STRUCT */
  struct user_struct user_data;
  
  /* TYPE_LANG_STRUCT */
  struct lang_struct *lang_data;
};

/* Forward declarations for TYPE_UNDEFINED */
struct forward_declared;
extern struct forward_declared *external_ref;

/* Self-referential structures */
struct recursive_struct {
  int value;
  struct recursive_struct *GTY((skip)) next;  /* skip to avoid infinite recursion */
  struct recursive_struct *GTY((skip)) prev;
};

/* Template-like structure (simulating C++ templates in C) */
#define DEFINE_CONTAINER(TYPE, NAME) \
  struct NAME { \
    TYPE data; \
    struct NAME *next; \
  }

DEFINE_CONTAINER(int, int_container);
DEFINE_CONTAINER(char *, string_container);

/* Variable declarations to ensure they're processed */
extern struct master_container GTY((root)) *root_container;
extern struct recursive_struct GTY((chain_next ("%h.next"))) *chain_root;
