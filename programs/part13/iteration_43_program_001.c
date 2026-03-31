/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

enum my_enum {
  E1,
  E2,
  E3
};

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* void_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char small_array[5];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
  void *data;
  int id;
  callback_type callback;
};

/* TYPE_UNION: Union type */
union my_union {
  int a;
  float b;
  void *c;
  callback_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr_field;
  
  /* TYPE_ARRAY (fixed size) */
  int array_field[8];
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_SCALAR */
  int count;
  enum my_enum status;
  
  /* TYPE_CALLBACK */
  callback_type handler;
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* Nested TYPE_STRUCT */
  struct plain_struct nested_struct;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_struct * GTY((skip)) user_ptr;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific *lang_ptr;
};

/* Another GC-tracked struct with flexible array member */
struct GTY(()) flex_struct {
  int length;
  char data GTY((length ("%h.length"))) [];
};

/* Union with GTY marker */
union GTY((desc ("%0.type"))) tagged_union {
  int type;
  struct complex_struct * GTY((tag ("0"))) cs;
  struct flex_struct * GTY((tag ("1"))) fs;
};

/* Struct containing array of pointers */
struct GTY(()) pointer_array_struct {
  int size;
  void * GTY((length ("%h.size"))) pointers[];
};

/* Callback function type with complex signature */
typedef void (*complex_callback)(
  struct complex_struct *cs,
  union tagged_union *tu,
  int count,
  const char *message
);

/* Final top-level structure that references everything */
struct GTY(()) master_struct {
  /* TYPE_STRUCT reference */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT reference */
  struct user_struct *user;
  
  /* TYPE_UNION */
  union my_union data;
  
  /* TYPE_POINTER to various types */
  struct complex_struct *complex;
  struct flex_struct *flex;
  struct pointer_array_struct *parray;
  
  /* TYPE_ARRAY of scalars */
  int scores[5];
  
  /* TYPE_ARRAY of pointers */
  string_type GTY((length ("%h.string_count"))) strings[10];
  int string_count;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific lang;
  
  /* TYPE_SCALAR */
  scalar_int id;
  enum my_enum mode;
  
  /* TYPE_STRING */
  const char *description;
  
  /* TYPE_CALLBACK */
  complex_callback notify;
  
  /* TYPE_UNDEFINED pointer */
  struct opaque_struct *opaque;
};

/* Additional TYPE_ARRAY variations */
typedef struct complex_struct complex_array[3];
typedef union my_union union_array[2][2];

/* TYPE_POINTER to callback */
typedef callback_type *callback_ptr;

/* Void pointer array */
typedef void *void_ptr_array[];

/* Const pointer */
typedef const int *const_int_ptr;

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Function returning pointer */
typedef struct master_struct *(*factory_fn)(int);

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
  void *data;
  int refcount;
};

/* Global variable declarations for completeness */
extern struct master_struct * GTY((root)) global_master;
extern complex_array GTY((length ("3"))) global_complex_array;
