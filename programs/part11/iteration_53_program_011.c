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
  int user_data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char c;
};

/* TYPE_POINTER - struct with pointer member */
struct with_ptr {
  int *p;
  struct my_struct *next;
};

/* TYPE_ARRAY - struct with array member */
struct with_array {
  int arr[10];
  char name[32];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%1"), tag("TREE_CODE"))) tree_node {
  int code;
  union tree_node *chain;
};

/* TYPE_SCALAR - typedef of fundamental type */
typedef int my_scalar;
typedef unsigned long size_type;

/* TYPE_STRING - char pointer for strings */
struct with_string {
  char *name;
  const char *path;
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, void*);
struct with_callback {
  callback_func handler;
  void (*cleanup)(struct with_string*);
};

/* Additional complex types to ensure coverage */
struct complex_type {
  struct my_struct base;
  union my_union data;
  struct with_ptr *ptr_field;
  int (*compare)(const void*, const void*);
  char *description;
  int flags[4];
};

/* Linked list example */
struct list_node {
  int value;
  struct list_node *GTY((skip)) next;  /* skip in GC marking */
};

/* Nested structures */
struct container {
  struct {
    int x;
    int y;
  } point;
  union {
    int int_val;
    float float_val;
  } data;
};
