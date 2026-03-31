/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque;
typedef struct opaque undefined_type_t;

/* TYPE_SCALAR - fundamental type */
typedef int my_scalar;

/* TYPE_STRING - string type */
typedef const char *my_string;

/* TYPE_POINTER - pointer type */
typedef int *int_ptr;

/* TYPE_ARRAY - array type */
typedef int int_array[10];

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  float b;
  int_ptr c;
};

/* TYPE_UNION - union type */
union my_union {
  int i;
  float f;
  char *s;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int data;
  void (*callback)(void);
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, float);

/* TYPE_LANG_STRUCT - simulate GCC language-specific type */
struct GTY((desc("%1"), tag("TREE"))) lang_struct {
  int code;
  union my_union u;
};

/* Complex nested types to ensure traversal */
struct container {
  struct my_struct s;
  union my_union u;
  int_array arr;
  my_string str;
  callback_func cb;
  struct lang_struct *lang;
};

/* Another level of nesting */
struct outer_container {
  struct container c;
  struct outer_container *next;  /* linked list */
  undefined_type_t *opaque_ptr;  /* pointer to undefined type */
};
