/* Test types for gengtype coverage - covering all type_kind enum cases */

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef long my_long_scalar;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef char *my_mutable_string;
typedef const char *const my_const_string;

/* TYPE_STRUCT: Complete C structs */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  my_string field3;
};

struct GTY(()) another_struct {
  double d;
  float f;
  struct my_struct *next;
};

struct GTY((chain_next("%h.next"))) chained_struct {
  int value;
  struct chained_struct *next;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct {
  int data;
  void *opaque;
};

struct GTY((user)) another_user_struct {
  long id;
  const char *name;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  double d;
  void *p;
};

union GTY(()) another_union {
  long l;
  float f;
  char c;
};

/* TYPE_POINTER: Pointer types */
typedef my_struct *my_pointer;
typedef another_struct *another_pointer;
typedef void *generic_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef my_struct struct_array[5];
typedef const char *string_array[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int);
typedef int (*another_callback)(const char *, int);
typedef void (*void_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next="%h.next")) lang_struct_type {
  int tag;
  union {
    int ival;
    double dval;
    const char *sval;
  } GTY((desc("%1.tag"))) u;
  struct lang_struct_type *next;
};

struct GTY((desc("%0.kind"), skip)) another_lang_struct {
  enum { KIND_A, KIND_B, KIND_C } kind;
  union {
    struct my_struct *a;
    struct another_struct *b;
    my_callback c;
  } GTY((desc("%0.kind"))) value;
};

/* Additional complex types to ensure thorough coverage */
struct GTY(()) complex_container {
  my_array arr;
  my_pointer ptr;
  my_callback cb;
  union my_union un;
};

/* Nested struct with various type kinds */
struct GTY(()) nested_types {
  /* scalar */
  int count;
  
  /* string */
  const char *name;
  
  /* struct */
  struct my_struct embedded;
  
  /* pointer */
  another_struct *ptr;
  
  /* array */
  int numbers[5];
  
  /* callback */
  another_callback handler;
  
  /* union */
  union another_union choice;
};

/* Chain of structures for traversal */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) double_linked {
  int id;
  struct double_linked *next;
  struct double_linked *prev;
};

/* Skip annotation test */
struct GTY((skip)) skipped_struct {
  int internal_data;
  void *private_ptr;
};

/* Variable length array in struct */
struct GTY(()) var_struct {
  int len;
  int data[1];
};
