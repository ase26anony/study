/* Test coverage file for gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_UNDEFINED: void type */
typedef void void_type;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_SCALAR: Enum type */
enum my_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_STRING: String types */
typedef const char *string_type;
typedef char *mutable_string_type;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, const char*);
typedef void (*callback_with_arg)(struct opaque_struct*);

/* TYPE_POINTER: Simple pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef const void* const_generic_ptr;

/* TYPE_ARRAY: Fixed-size array */
typedef int fixed_array[10];
typedef char char_array[256];

/* TYPE_ARRAY: Multi-dimensional array */
typedef int matrix[3][3];

/* TYPE_STRUCT: Plain C struct (no GC tracking) */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_UNION: Plain C union */
union plain_union {
  int as_int;
  float as_float;
  void* as_ptr;
  char as_char;
};

/* TYPE_USER_STRUCT: GC-aware user struct */
struct GTY((user)) user_struct {
  void * GTY((skip)) data;
  int id;
};

/* TYPE_STRUCT with nested types */
struct nested_struct {
  int scalar_field;
  int_ptr pointer_field;
  fixed_array array_field;
  enum my_enum enum_field;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void* GTY((length("strlen(data) + 1"))) data;
};

/* Complex GC-tracked structure with various field types */
struct GTY(()) complex_gc_struct {
  /* TYPE_POINTER */
  void* GTY((skip)) ptr_field;
  
  /* TYPE_STRING */
  const char* GTY((length("strlen(string_field) + 1"))) string_field;
  
  /* TYPE_ARRAY (embedded) */
  int embedded_array[5];
  
  /* TYPE_SCALAR */
  int scalar_field;
  enum my_enum enum_field;
  
  /* TYPE_CALLBACK */
  simple_callback callback_field;
  
  /* Nested TYPE_STRUCT */
  struct nested_struct nested;
  
  /* Pointer to TYPE_UNION */
  union plain_union* union_ptr;
};

/* TYPE_ARRAY with GC tracking */
struct GTY(()) array_container {
  int GTY((length("array_length"))) *variable_array;
  int array_length;
  
  /* Flexible array member */
  char GTY((length("strlen(flex_array) + 1"))) flex_array[];
};

/* Union with GC tracking */
union GTY(()) gc_union {
  struct complex_gc_struct* as_struct;
  struct array_container* as_array;
  string_type as_string;
};

/* Another callback type using GC types */
typedef void (*gc_callback)(struct complex_gc_struct*, union gc_union*);

/* Container struct using all types */
struct GTY(()) master_container {
  /* TYPE_USER_STRUCT */
  struct user_struct* user;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific* lang;
  
  /* TYPE_POINTER to various types */
  struct opaque_struct* opaque_ptr;
  struct plain_struct* plain_ptr;
  
  /* TYPE_ARRAY of pointers */
  struct complex_gc_struct* GTY((length("ptr_count"))) *ptr_array;
  int ptr_count;
  
  /* TYPE_UNION */
  union gc_union data_union;
  
  /* TYPE_CALLBACK */
  gc_callback handler;
  
  /* TYPE_STRING array */
  const char* GTY((length("name_count"))) names[];
  int name_count;
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* Typedef for undefined type */
typedef struct never_defined* undefined_ptr;

/* Array of undefined pointers */
struct GTY(()) undefined_container {
  undefined_ptr GTY((length("count"))) *items;
  int count;
};

/* Mixed struct with bitfields (scalar special case) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int value : 8;
  unsigned int : 5; /* unnamed bitfield */
  unsigned int last : 16;
};

/* Struct with function pointer returning pointer */
struct GTY(()) callback_container {
  struct complex_gc_struct* (* GTY((callback))) allocator(int size);
  void (* GTY((callback))) deallocator(struct complex_gc_struct*);
};

/* Ensure we have TYPE_NONE case covered (gcc_unreachable) */
/* This is the default case in the switch, should never be reached */
/* We don't need to explicitly create a TYPE_NONE value */

/* Additional edge cases */

/* Const pointer to const */
typedef const struct complex_gc_struct* const* const_double_ptr;

/* Array of function pointers */
typedef void (*func_array[5])(void);

/* Struct containing array of structs */
struct GTY(()) struct_array_container {
  struct nested_struct elements[10];
  int count;
};

/* Union containing different pointer types */
union GTY(()) pointer_union {
  int* int_ptr;
  char** string_ptr_ptr;
  struct complex_gc_struct** struct_ptr_ptr;
};

/* Typedef chain */
typedef struct plain_struct plain_t;
typedef plain_t* plain_ptr_t;
typedef plain_ptr_t* plain_double_ptr_t;

/* Empty struct (edge case) */
struct GTY(()) empty_struct {
  /* No fields */
};

/* Struct with only scalar fields */
struct GTY(()) scalar_only {
  int a;
  float b;
  double c;
  char d;
  enum my_enum e;
};

/* Complete the opaque struct declaration if needed elsewhere */
/* Leaving it undefined maintains TYPE_UNDEFINED status */

/* Final master type that references everything */
struct GTY(()) coverage_master {
  struct master_container* container;
  struct undefined_container* undefined;
  struct bitfield_struct* bitfields;
  struct callback_container* callbacks;
  struct struct_array_container* struct_array;
  union pointer_union* ptr_union;
  struct empty_struct* empty;
  struct scalar_only* scalars;
  const_double_ptr const_ptr;
  func_array functions;
};
