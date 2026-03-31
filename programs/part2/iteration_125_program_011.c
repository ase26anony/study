/* Test types for gengtype coverage - covering all type_kind cases */

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_scalar2;
typedef double my_scalar3;

/* TYPE_STRING: String typedefs */
typedef const char *my_string;
typedef char *my_string2;
typedef const char * const my_string3;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  int field1;
  double field2;
  my_string field3;
};

struct GTY((skip)) another_struct {
  long long data;
  struct my_struct *next;
};

struct GTY((chain_next = "%h.next")) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  void *data;
  int tag;
};

struct GTY((user)) another_user_struct {
  int x, y, z;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int as_int;
  double as_double;
  void *as_ptr;
};

union GTY((desc("%0.as_int"))) tagged_union {
  int as_int;
  float as_float;
  struct my_struct *as_struct;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef my_scalar *scalar_pointer;

/* TYPE_ARRAY: Array typedefs */
typedef int my_array[10];
typedef struct my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(struct my_struct *, my_string);
typedef void (*void_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
};

struct GTY((desc("0"), tag("1"))) another_lang_struct {
  int tag;
  union {
    int as_int;
    double as_double;
  } GTY((desc("%1.tag"))) u;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  my_array arr;
  my_callback cb;
  union my_union uni;
  struct user_struct user;
};

/* Nested pointer/array combinations */
typedef struct my_struct *pointer_array[4];
typedef int (*callback_array[2])(void);

/* Mixed struct with various field types */
struct GTY(()) mixed_types {
  /* scalar */
  int count;
  
  /* string */
  const char *name;
  
  /* pointer */
  struct mixed_types *next;
  
  /* array */
  int values[8];
  
  /* callback */
  my_callback handler;
  
  /* union */
  union {
    int i;
    void *p;
  } choice;
  
  /* nested struct */
  struct {
    int x, y;
  } point;
};
