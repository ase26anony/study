/* test-coverage.gt - Test file to cover all TYPE_* cases in gengtype-state.cc */

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
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void*);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char small_array[5];

/* TYPE_STRUCT: Plain C struct without GTY markers */
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
  const char *d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr_field;
  
  /* TYPE_ARRAY (fixed size) */
  int array_field[20];
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_SCALAR */
  int count;
  enum my_enum status;
  
  /* TYPE_CALLBACK */
  callback_type handler;
  
  /* Nested TYPE_UNION */
  union my_union data;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_struct * GTY((skip)) user_data;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific *lang_ptr;
};

/* Another GC struct with flexible array member */
struct GTY(()) flexible_struct {
  int length;
  char data GTY((length ("%h.length")))[];
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
  int *int_ptr;
  void **void_ptr_ptr;
  struct complex_struct *struct_ptr;
  callback_type func_ptr;
};

/* Struct with array of pointers */
struct GTY(()) pointer_array_struct {
  void *pointers[10];
  callback_type handlers[5];
  string_type strings[3];
};

/* Type with nested anonymous struct/union */
struct GTY(()) nested_anonymous {
  union {
    int x;
    float y;
  } data;
  
  struct {
    int a;
    int b;
  } coords;
};

/* Forward reference chain */
struct forward_b;
struct forward_a {
  struct forward_b *next;
};

struct forward_b {
  struct forward_a *prev;
  int value;
};

/* Array of unions */
union mixed_data {
  int i;
  float f;
  void *p;
  const char *s;
};

struct GTY(()) array_container {
  union mixed_data items[100];
  int item_count;
};

/* Struct with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int value : 16;
};

/* Multiple levels of indirection */
typedef int ***triple_int_ptr;
typedef callback_type (*meta_callback)(callback_type);

/* Void type (contributes to TYPE_UNDEFINED) */
typedef void void_type;

/* Opaque pointer type */
typedef struct undefined_type* opaque_handle;

/* Struct with all basic scalar types */
struct GTY(()) all_scalars {
  char c;
  signed char sc;
  unsigned char uc;
  short s;
  unsigned short us;
  int i;
  unsigned int ui;
  long l;
  unsigned long ul;
  long long ll;
  unsigned long long ull;
  float f;
  double d;
  _Bool b;
};

/* Complete the forward declaration to avoid warnings */
struct opaque_struct {
  int defined_later;
  void *some_data;
};

/* Array of strings */
typedef const char *string_array[50];

/* Callback with complex signature */
typedef struct complex_struct* (*factory_callback)(
  int count,
  const char *name,
  void *context
);

/* Struct using the complex callback */
struct GTY(()) factory {
  factory_callback create;
  void *context;
  string_type type_name;
};

/* Union with struct members */
union GTY(()) struct_union {
  struct complex_struct cs;
  struct flexible_struct fs;
  struct all_scalars as;
};

/* Multi-dimensional array */
typedef int matrix[10][20];
typedef const char* string_matrix[5][5];

/* Final catch-all structure containing references to all types */
struct GTY(()) master_container {
  /* TYPE_STRUCT */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_struct *user;
  
  /* TYPE_UNION */
  union my_union u;
  
  /* TYPE_POINTER */
  void *generic_pointer;
  int_ptr int_pointer;
  
  /* TYPE_ARRAY */
  fixed_array numbers;
  char buffer[256];
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific *lang;
  
  /* TYPE_SCALAR */
  scalar_int s_int;
  enum myEnum current_enum;
  
  /* TYPE_STRING */
  string_type title;
  const char *description;
  
  /* TYPE_CALLBACK */
  callback_type on_event;
  factory_callback factory;
  
  /* TYPE_UNDEFINED reference */
  struct opaque_struct *opaque;
  
  /* Complex nested */
  struct complex_struct complex;
  struct flexible_struct *flex;
  
  /* Arrays of various types */
  union mixed_data data_array[10];
  callback_type callback_array[3];
  
  /* Multi-dimensional */
  matrix grid;
  
  /* Bitfields */
  struct bitfield_struct flags;
  
  /* All scalars */
  struct all_scalars scalars;
  
  /* Pointer arrays */
  struct pointer_array_struct *ptr_arrays;
  
  /* Anonymous */
  struct nested_anonymous anonymous;
  
  /* Forward reference chain */
  struct forward_a *chain;
  
  /* Union of structs */
  union struct_union su;
  
  /* Factory */
  struct factory producer;
  
  /* String array */
  string_array messages;
  
  /* Triple pointer */
  triple_int_ptr triple_ptr;
  
  /* Meta callback */
  meta_callback meta;
};
