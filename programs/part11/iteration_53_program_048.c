/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - opaque/incomplete type */
struct opaque_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  char b;
  double c;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char c;
  void *ptr;
};

/* TYPE_POINTER - struct with pointer member */
struct with_ptr {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void *void_ptr;
};

/* TYPE_ARRAY - struct with array members */
struct with_array {
  int arr[10];
  char str[50];
  struct my_struct structs[5];
};

/* TYPE_LANG_STRUCT - simulate GCC language-specific type */
struct GTY((desc("%1"))) lang_struct {
  int lang_specific;
  union my_union data;
};

/* TYPE_SCALAR - typedef of fundamental type */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING - char* string type */
struct with_string {
  char *name;
  const char *const_name;
  char *array_strings[3];
};

/* TYPE_CALLBACK - function pointer types */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(struct my_struct *, int);

struct with_callbacks {
  simple_callback cb1;
  complex_callback cb2;
  void (*void_cb)(int, char*);
};

/* TYPE_NONE should never appear in valid input */

/* Additional composite types to ensure coverage */
struct composite {
  struct my_struct s;
  union my_union u;
  struct with_ptr *next;
  struct with_array arr_data;
  my_scalar scalar_val;
  char *description;
  simple_callback handler;
};

/* Enum type (treated as scalar) */
enum my_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* Forward declarations for pointer chains */
struct forward_decl;
struct another_forward;

struct pointer_chain {
  struct forward_decl *fwd;
  struct another_forward *another;
  struct pointer_chain *self_ref;
};
