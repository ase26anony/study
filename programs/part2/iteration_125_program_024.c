/* Test types for gengtype coverage - covering all type_kind enum cases */

/* TYPE_UNDEFINED - incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR - simple scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_scalar2;
typedef double my_scalar3;

/* TYPE_STRING - string typedefs */
typedef const char *my_string;
typedef const char *my_string2;
typedef const char *my_string3;

/* TYPE_STRUCT - complete structs with GTY annotations */
struct GTY(()) my_struct {
  int field1;
  double field2;
  my_scalar field3;
};

struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_struct {
  int data;
  struct linked_struct * GTY((skip)) next;
  struct linked_struct *prev;
};

struct GTY((desc ("%1.type"))) tagged_struct {
  enum { TYPE_A, TYPE_B } type;
  union {
    int int_val;
    double dbl_val;
  } GTY((desc ("%0.type"))) value;
};

/* TYPE_USER_STRUCT - user-defined structs */
struct GTY((user)) user_struct {
  void *opaque_data;
  int user_tag;
};

struct GTY((user)) another_user_struct {
  long custom_field;
  void (*user_func)(void);
};

/* TYPE_UNION - union types */
union GTY(()) my_union {
  int int_val;
  double dbl_val;
  char * GTY((length ("strlen(%0.str_val) + 1"))) str_val;
};

union GTY((desc ("%0.tag"))) tagged_union {
  int tag;
  struct {
    int x, y;
  } point;
  struct {
    double radius;
  } circle;
};

/* TYPE_POINTER - pointer typedefs */
typedef my_struct *my_pointer;
typedef my_pointer *double_pointer;
typedef const my_struct *const_pointer;
typedef union my_union *union_pointer;

/* TYPE_ARRAY - array typedefs */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];
typedef int multi_array[2][3][4];

/* TYPE_CALLBACK - function pointer types */
typedef void (*my_callback)(int, double);
typedef int (*filter_func)(const char *);
typedef my_struct *(*factory_func)(void);
typedef void (*void_func)(void);

/* TYPE_LANG_STRUCT - language-specific structs */
struct GTY((desc ("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc ("%0.kind"), tag ("true"))) another_lang_struct {
  enum lang_kind { LANG_A, LANG_B } kind;
  union {
    int int_val;
    void *ptr_val;
  } GTY((desc ("%0.kind"))) data;
};

/* Additional complex types to ensure thorough coverage */

/* Nested struct with array */
struct GTY(()) container_struct {
  my_array items;
  my_callback processor;
  union my_union variant;
};

/* Struct with callback field */
struct GTY(()) callback_container {
  filter_func filter;
  factory_func create;
  void_func cleanup;
};

/* Pointer to array */
typedef my_array *array_pointer;

/* Array of pointers */
typedef my_struct *pointer_array[8];

/* Union containing struct */
union GTY(()) struct_in_union {
  struct {
    int a, b;
  } pair;
  struct {
    double x, y, z;
  } point3d;
};

/* Struct with nested anonymous struct */
struct GTY(()) nested_anon {
  struct {
    int depth;
    char *name;
  } inner;
  int outer_field;
};

/* Variable length array struct (for length annotation) */
struct GTY(()) var_len_struct {
  int count;
  int data[1];
};

/* Skip-annotated struct */
struct GTY((skip)) skipped_struct {
  void *internal_data;
  int private_field;
};

/* Optional struct (for maybe_undef annotation) */
struct GTY((maybe_undef)) optional_struct {
  int present;
  my_struct * GTY((tag ("0"))) data;
};
