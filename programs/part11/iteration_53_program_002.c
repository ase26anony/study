/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - opaque/incomplete type */
struct opaque_struct;
typedef struct opaque_struct undefined_type_t;

/* TYPE_SCALAR - fundamental type */
typedef int my_scalar_t;

/* TYPE_STRING - string type */
typedef const char *string_type_t;

/* TYPE_POINTER - pointer type */
typedef int *int_ptr_t;

/* TYPE_ARRAY - array type */
typedef int array_type_t[10];

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  float b;
  my_scalar_t c;
};

/* TYPE_UNION - union type */
union my_union {
  int i;
  float f;
  double d;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_t)(int, const char*);

/* TYPE_LANG_STRUCT - language-specific struct */
struct GTY((desc("%1"), tag("TREE"), chain_next("%h.next"))) tree_node {
  int code;
  struct tree_node *next;
};

/* Complex types combining multiple kinds */
struct container_struct {
  my_scalar_t scalar;          /* TYPE_SCALAR */
  string_type_t str;           /* TYPE_STRING */
  int_ptr_t ptr;               /* TYPE_POINTER */
  array_type_t arr;            /* TYPE_ARRAY */
  struct my_struct nested;     /* TYPE_STRUCT */
  union my_union uni;          /* TYPE_UNION */
  callback_t callback;         /* TYPE_CALLBACK */
  struct tree_node *lang_node; /* TYPE_LANG_STRUCT */
};

/* Another level of indirection */
typedef struct container_struct *container_ptr_t;
typedef container_ptr_t container_ptr_array_t[5];
