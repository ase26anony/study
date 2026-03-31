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

/* TYPE_POINTER */
struct with_pointers {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void *void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int fixed_arr[10];
  char str_arr[20];
  struct my_struct struct_arr[5];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%0"))) lang_struct {
  int lang_specific;
  void *tree_code;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long size_type;

/* TYPE_STRING */
struct with_strings {
  char *name;
  const char *const_name;
  char filename[256];
};

/* TYPE_CALLBACK */
typedef void (*callback_func)(int, void*);
typedef int (*compare_func)(const void*, const void*);

struct with_callbacks {
  callback_func cb;
  compare_func cmp;
  void (*simple_cb)(void);
};

/* Composite type using all kinds */
struct GTY(()) composite_type {
  struct my_struct regular;      /* TYPE_STRUCT */
  union my_union variant;        /* TYPE_UNION */
  int *ptr;                      /* TYPE_POINTER */
  int array[5];                  /* TYPE_ARRAY */
  char *str;                     /* TYPE_STRING */
  callback_func handler;         /* TYPE_CALLBACK */
  my_scalar value;               /* TYPE_SCALAR */
};

/* Forward declarations for pointer chains */
struct forward_decl;
struct another_forward;

struct pointer_chain {
  struct forward_decl *fwd;
  struct another_forward *another;
};

/* Enum type */
typedef enum {
  RED,
  GREEN,
  BLUE
} color_enum;

/* Function pointer variations */
typedef void (*simple_handler)(void);
typedef int (*complex_handler)(struct my_struct*, union my_union*, int);
