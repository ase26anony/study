/* Test coverage for gengtype-state.cc switch statement cases */

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
typedef int (*another_callback)(const char*, void*);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

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
  string_type lang_name;
  enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GTY structure containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER field */
  void *ptr_field;
  
  /* TYPE_ARRAY field */
  int array_field[8];
  
  /* TYPE_STRING field */
  const char *name;
  
  /* TYPE_SCALAR fields */
  int int_field;
  enum my_enum enum_field;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
  
  /* Nested TYPE_STRUCT */
  struct plain_struct nested_plain;
  
  /* Pointer to TYPE_UNION */
  union my_union *union_ptr;
  
  /* Flexible array member (TYPE_ARRAY) */
  int flexible_array[];
};

/* Another GTY structure with nested language struct */
struct GTY(()) container_struct {
  struct lang_specific lang_item;
  struct complex_struct *complex_ptr;
  generic_ptr data;
  fixed_array numbers;
};

/* Union containing various pointer types */
union pointer_union {
  int_ptr int_pointer;
  generic_ptr void_pointer;
  string_type string_pointer;
  callback_type callback_pointer;
  struct complex_struct *struct_pointer;
};

/* Array of pointers */
typedef void* pointer_array[20];

/* Structure with array of callbacks */
struct GTY(()) callback_container {
  callback_type callbacks[5];
  int callback_count;
};

/* Forward declaration for circular reference */
struct forward_declared;

/* Structure with pointer to forward-declared type */
struct GTY(()) circular_struct {
  struct forward_declared *next;
  int value;
};

/* Now define the forward-declared structure */
struct GTY(()) forward_declared {
  struct circular_struct *prev;
  string_type name;
};

/* Enumeration type definition */
typedef enum color {
  RED,
  GREEN,
  BLUE
} color_t;

/* Structure using the enumeration */
struct GTY(()) color_struct {
  color_t current_color;
  color_t palette[3];
};

/* Mixed structure with all types */
struct GTY(()) all_types_struct {
  /* SCALAR */
  int scalar_int;
  enum my_enum scalar_enum;
  
  /* POINTER */
  void *pointer;
  int_ptr int_pointer;
  
  /* ARRAY */
  int fixed_size[5];
  char variable_length[];
  
  /* STRING */
  const char *string_field;
  
  /* CALLBACK */
  callback_type callback_field;
  
  /* STRUCT (nested) */
  struct plain_struct nested_struct;
  
  /* UNION */
  union my_union data_union;
  
  /* USER STRUCT */
  struct user_struct *user_struct_ptr;
  
  /* LANG STRUCT */
  struct lang_specific lang_struct;
};

/* Typedef for a function type (another callback variant) */
typedef void (*void_func)(void);

/* Structure containing function pointer array */
struct GTY(()) func_table {
  void_func functions[10];
  const char *function_names[10];
};

/* End of test coverage definitions */
