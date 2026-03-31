/* test-coverage.gt - Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

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
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char small_array[5];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user structure */
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
  string_type lang_name;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void_ptr generic_pointer;
  
  /* TYPE_ARRAY (fixed size) */
  fixed_array numbers;
  
  /* TYPE_STRING */
  string_type name;
  
  /* TYPE_CALLBACK */
  callback_type handler;
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* TYPE_POINTER to another struct */
  struct plain_struct* plain_ptr;
  
  /* TYPE_ARRAY of pointers */
  int_ptr* pointer_array[8];
  
  /* Flexible array member (variable-length array) */
  int flexible_array GTY((length("%h.flexible_count")));
  int flexible_count;
};

/* Another GTY struct with nested structures */
struct GTY(()) container_struct {
  /* TYPE_STRUCT (embedded) */
  struct plain_struct embedded;
  
  /* TYPE_USER_STRUCT pointer */
  struct user_struct* user_data;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific lang_item;
  
  /* TYPE_ARRAY of structs */
  struct plain_struct struct_array[3];
  
  /* TYPE_UNION */
  union my_union choice;
  
  /* TYPE_CALLBACK array */
  callback_type callbacks[2];
  
  /* Nested pointer to same type (recursive) */
  struct container_struct* next;
};

/* Union with GTY marker */
union GTY((desc ("%0.type"))) tagged_union {
  int type;
  struct GTY((tag ("0"))) {
    int int_value;
  } as_int;
  struct GTY((tag ("1"))) {
    string_type string_value;
  } as_string;
  struct GTY((tag ("2"))) {
    callback_type func_value;
  } as_func;
};

/* Additional test cases for edge scenarios */

/* TYPE_POINTER to callback */
typedef callback_type* callback_ptr;

/* TYPE_ARRAY of strings */
typedef string_type string_array[5];

/* Struct with array of unions */
struct GTY(()) union_container {
  union my_union unions[4];
  int count;
};

/* Void pointer typedef (could contribute to TYPE_UNDEFINED/pointer) */
typedef void void_type;

/* Struct with bitfields (scalar handling) */
struct bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 3;
  unsigned int flag3 : 4;
};

/* Ensure we have a typedef for a struct type */
typedef struct plain_struct plain_struct_t;

/* Global variable declarations to ensure processing */
extern int global_scalar;
extern string_type global_string;
extern struct complex_struct* global_complex;
