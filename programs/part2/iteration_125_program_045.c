/* Test types for gengtype coverage - covering all type_kind cases */

/* TYPE_UNDEFINED - incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR - simple scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_scalar2;
typedef double my_scalar3;

/* TYPE_STRING - string types */
typedef const char *my_string;
typedef char *my_mutable_string;
typedef const char * const my_const_string;

/* TYPE_STRUCT - complete C structs */
struct GTY(()) my_struct {
  int field1;
  double field2;
  my_string field3;
};

struct GTY((skip)) another_struct {
  long data;
  struct my_struct *next;
};

struct GTY((chain_next = "%h.next")) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

/* TYPE_USER_STRUCT - user-defined structs */
struct GTY((user)) user_struct_type {
  void *data;
  int tag;
};

struct GTY((user)) another_user_struct {
  long custom_data;
  int flags;
};

/* TYPE_UNION - union types */
union GTY(()) my_union {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%0"))) tagged_union {
  int type;
  struct my_struct as_struct;
  union my_union as_union;
};

/* TYPE_POINTER - pointer types */
typedef struct my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef my_scalar *scalar_pointer;

/* TYPE_ARRAY - array types */
typedef int my_array[10];
typedef struct my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK - function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(struct my_struct *, my_string);
typedef void (*void_callback)(void);

/* TYPE_LANG_STRUCT - language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("%0"), tag("LANG_TYPE"))) another_lang_struct {
  int type_tag;
  union tagged_union data;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  struct my_struct nested_struct;
  union my_union nested_union;
  my_array fixed_array;
  my_callback callback_field;
  struct GTY((user)) user_struct_type *user_ptr;
};

/* Nested pointer/array combinations */
typedef struct my_struct *pointer_array[4];
typedef my_callback callback_array[2];

/* Anonymous struct/union in typedef */
typedef struct GTY(()) {
  int x;
  int y;
} anonymous_struct;

typedef union GTY(()) {
  int i;
  float f;
} anonymous_union;
