/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  double b;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  void *data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char c;
};

/* TYPE_POINTER */
struct with_pointers {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void *void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int int_arr[10];
  char char_arr[20];
  struct my_struct struct_arr[5];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("tree_node"))) tree_node {
  int code;
  union tree_node_u *u;
};

union tree_node_u {
  int ival;
  double dval;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long size_type;

/* TYPE_STRING */
struct with_strings {
  char *str;
  const char *cstr;
  char name[50];
};

/* TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
typedef void (*void_callback)(void);

struct with_callbacks {
  callback_func handler;
  void_callback cleanup;
};

/* Nested types to ensure thorough processing */
struct container {
  struct my_struct s;
  union my_union u;
  struct with_pointers *wp;
  struct with_arrays wa;
  struct with_strings ws;
  struct with_callbacks wc;
};

/* Forward declarations for pointer chains */
struct forward_decl;
struct another_forward;

struct pointer_chain {
  struct forward_decl *fwd;
  struct another_forward *another;
};

struct forward_decl {
  int value;
};

struct another_forward {
  char *name;
};

/* Enum type */
typedef enum {
  RED,
  GREEN,
  BLUE
} color_enum;

/* Function pointer arrays */
typedef void (*func_array[10])(void);

/* Complex nested structure */
struct complex_nested {
  struct {
    int x;
    int y;
  } point;
  
  union {
    struct {
      int width;
      int height;
    } rect;
    struct {
      int radius;
    } circle;
  } shape;
  
  struct complex_nested *next;
};
