/* Test types for gengtype coverage - covering all type_kind cases */

/* TYPE_UNDEFINED: Incomplete type */
struct undefined_type;
struct another_undefined;

/* TYPE_SCALAR: Simple scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_scalar2;
typedef double my_scalar3;

/* TYPE_STRING: String types */
typedef const char *my_string;
typedef char *my_string2;
typedef const char * const my_string3;

/* TYPE_STRUCT: Complete C structs */
struct GTY((skip)) my_struct {
  int field1;
  double field2;
  const char *field3;
};

struct GTY((chain_next = "%h.next")) linked_struct {
  int data;
  struct linked_struct * GTY((skip)) next;
};

struct GTY(()) complex_struct {
  struct my_struct nested;
  my_scalar scalar_field;
  my_string string_field;
};

/* TYPE_USER_STRUCT: User-defined structs */
struct GTY((user)) user_struct_type {
  void *user_data;
  int user_id;
};

struct GTY((user)) another_user_struct {
  long custom_field;
  struct user_struct_type *related;
};

/* TYPE_UNION: Union types */
union GTY((desc("%0"))) my_union {
  int int_val;
  double double_val;
  const char *string_val;
  struct my_struct *struct_ptr;
};

union GTY(()) tagged_union {
  int type_tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    float radius;
    float angle;
  } polar;
};

/* TYPE_POINTER: Pointer types */
typedef struct my_struct *my_pointer;
typedef union my_union *union_pointer;
typedef my_scalar *scalar_pointer;
typedef const struct complex_struct *const_struct_pointer;

/* TYPE_ARRAY: Array types */
typedef int my_array[10];
typedef struct my_struct struct_array[5];
typedef const char *string_array[20];
typedef int multi_dim_array[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*my_callback)(int, const char*);
typedef int (*compare_func)(const void *, const void *);
typedef struct my_struct *(*allocator_func)(size_t);
typedef void (*void_func)(void);

/* TYPE_LANG_STRUCT: Language-specific structs */
struct GTY((desc("%1"), chain_next = "%h.next")) lang_struct_type {
  int lang_specific;
  struct lang_struct_type *next;
  void * GTY((skip)) lang_data;
};

struct GTY((desc("%0"), tag("LANG_NODE"))) lang_node {
  enum { LANG_EXPR, LANG_STMT, LANG_DECL } kind;
  union {
    struct { int op; struct lang_node *kids[2]; } expr;
    struct { int type; const char *text; } stmt;
    struct { const char *name; int line; } decl;
  } u;
};

/* Additional mixed types to ensure thorough coverage */
struct GTY(()) container_struct {
  my_array numbers;
  my_string name;
  my_callback handler;
  union my_union data;
  struct lang_struct_type *lang_list;
};

/* Pointer to callback */
typedef my_callback (*callback_getter)(void);

/* Array of pointers */
typedef struct my_struct *struct_ptr_array[8];

/* Union containing array */
union GTY(()) array_union {
  my_array ints;
  string_array strings;
};

/* Struct with all type kinds */
struct GTY((skip)) all_types_struct {
  /* scalar */
  my_scalar scalar;
  
  /* string */
  my_string str;
  
  /* struct */
  struct my_struct nested_struct;
  
  /* pointer */
  my_pointer ptr;
  
  /* array */
  my_array arr;
  
  /* callback */
  my_callback cb;
  
  /* union */
  union my_union uni;
};
