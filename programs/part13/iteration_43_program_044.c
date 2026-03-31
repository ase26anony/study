/* test-coverage.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

enum my_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
};

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void*);

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

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
  void *data;
  int id;
  callback_type callback;
};

/* TYPE_UNION: Union type */
union my_union {
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[20];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GTY struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER field */
  void *ptr_field;
  
  /* TYPE_ARRAY field */
  int array_field[5];
  
  /* TYPE_STRING field */
  const char *name;
  
  /* TYPE_SCALAR fields */
  int count;
  enum my_enum status;
  
  /* Nested TYPE_UNION */
  union my_union data;
  
  /* TYPE_CALLBACK field */
  callback_type handler;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_struct * GTY((skip)) user_data;
  
  /* Flexible array member (TYPE_ARRAY) */
  int flexible_array[];
};

/* Another GTY struct with pointer chain */
struct GTY(()) container {
  struct complex_struct * GTY((tag("0"))) item;
  struct container * GTY((tag("1"))) next;
  int id;
  string_type description;
};

/* Union containing structs */
union GTY(()) mixed_union {
  struct complex_struct cs;
  struct container *cont;
  int_ptr numbers;
  fixed_array values;
};

/* Typedef for a callback that returns a pointer */
typedef struct complex_struct* (*factory_callback)(int, string_type);

/* Struct using the callback typedef */
struct GTY(()) callback_container {
  factory_callback create;
  void (*destroy)(struct complex_struct *);
  int max_items;
};

/* Array of pointers */
typedef struct container *container_array[10];

/* Struct with nested array of structs */
struct GTY(()) nested_array_struct {
  struct complex_struct items[3];
  container_array containers;
  callback_type callbacks[5];
};

/* Forward declaration for circular reference */
struct forward_declared;

/* Struct with circular reference */
struct GTY(()) circular_struct {
  int value;
  struct circular_struct * GTY((tag("0"))) self_ref;
  struct forward_declared * GTY((tag("1"))) forward_ref;
};

/* Definition of forward declared struct */
struct GTY(()) forward_declared {
  char data[50];
  struct circular_struct * GTY((tag("0"))) back_ref;
};

/* Enum-union combination */
union GTY(()) enum_union {
  enum my_enum e;
  int i;
  float f;
};

/* Struct with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int value : 16;
};

/* Type with const qualifier */
typedef const int const_int;
typedef const struct complex_struct *const_complex_ptr;

/* Void pointer callback */
typedef void (*void_callback)(void);

/* Empty struct */
struct GTY(()) empty_struct {
  /* No fields */
};

/* Union with only scalars */
union scalar_union {
  int i;
  long l;
  double d;
};

/* Final catch-all structure containing one of everything */
struct GTY(()) master_struct {
  /* TYPE_STRUCT (nested) */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_struct *user;
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific *lang;
  
  /* TYPE_POINTER */
  void *generic_pointer;
  
  /* TYPE_ARRAY */
  int number_array[20];
  
  /* TYPE_SCALAR */
  int counter;
  enum my_enum current_state;
  
  /* TYPE_STRING */
  const char *title;
  
  /* TYPE_CALLBACK */
  callback_type notify;
  
  /* Complex nested type */
  struct complex_struct complex;
  
  /* Container */
  struct container *items;
  
  /* Mixed union */
  union mixed_union mixed;
  
  /* Callback container */
  struct callback_container *factory;
  
  /* Nested array struct */
  struct nested_array_struct nested;
  
  /* Circular reference */
  struct circular_struct *circular;
  
  /* Enum union */
  union enum_union enum_data;
  
  /* Bitfield struct */
  struct bitfield_struct flags;
  
  /* Const pointer */
  const_complex_ptr const_ptr;
  
  /* Void callback */
  void_callback cleanup;
  
  /* Empty struct */
  struct empty_struct empty;
  
  /* Scalar union */
  union scalar_union scalars;
  
  /* Flexible array of strings */
  const char *string_list[];
};
