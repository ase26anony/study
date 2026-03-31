/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - opaque/incomplete type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_SCALAR - fundamental type */
typedef int my_scalar;
typedef unsigned long scalar2;

/* TYPE_STRING - string type */
typedef const char *my_string;

/* TYPE_POINTER - pointer type */
struct pointer_container {
  int *int_ptr;
  struct opaque_struct *opaque_ptr;
  void *generic_ptr;
};

/* TYPE_ARRAY - array type */
struct array_container {
  int fixed_array[10];
  int multi_array[5][5];
  char string_array[20];
};

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int field1;
  double field2;
  my_scalar field3;
};

/* TYPE_UNION - union type */
union my_union {
  int as_int;
  double as_double;
  void *as_ptr;
  struct my_struct as_struct;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
  void *user_handle;
};

/* TYPE_LANG_STRUCT - language-specific struct */
struct GTY((desc("%0"))) lang_struct {
  int lang_tag;
  union my_union lang_data;
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, const char*);
typedef int (*compare_func)(const void*, const void*);

struct callback_container {
  callback_func cb;
  compare_func cmp;
  void (*simple_cb)(void);
};

/* Nested complex type to ensure traversal */
struct complex_container {
  struct my_struct nested_struct;
  union my_union nested_union;
  struct array_container nested_array;
  struct pointer_container nested_pointer;
  callback_func nested_callback;
  my_string nested_string;
};

/* Forward declarations for pointer chains */
struct forward_decl;
struct another_forward;

struct pointer_chain {
  struct forward_decl *fwd1;
  struct another_forward *fwd2;
  struct pointer_chain *self_ptr;
};

/* Enum type */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum;

struct enum_container {
  my_enum choice;
  int value;
};

/* Variable length array (simulated) */
struct var_container {
  int length;
  int data[1];  /* Flexible array member style */
};

/* Bitfield structure */
struct bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  unsigned int padding : 26;
};

/* Complete the forward declarations */
struct forward_decl {
  int data;
  struct another_forward *link;
};

struct another_forward {
  char name[20];
  struct forward_decl *backlink;
};
