/* Test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
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
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* void_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array_type[10];
typedef char string_array[20];

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
};

/* TYPE_UNION: Union type */
union my_union {
  int int_val;
  float float_val;
  void *ptr_val;
  char str_val[16];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), desc("language_specific"))) lang_specific {
  int lang_field;
  void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr_field;
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_ARRAY (fixed size) */
  int numbers[5];
  
  /* TYPE_UNION */
  union my_union data_union;
  
  /* TYPE_CALLBACK */
  callback_type callback;
  
  /* TYPE_SCALAR */
  int counter;
  enum my_enum status;
  
  /* TYPE_POINTER to another GTY struct */
  struct GTY(()) inner_struct *inner;
  
  /* TYPE_ARRAY of pointers */
  void *ptr_array[8];
};

/* Another GTY struct for pointer relationships */
struct GTY(()) inner_struct {
  int id;
  struct complex_struct *parent;  /* TYPE_POINTER back to parent */
  string_type description;         /* TYPE_STRING */
};

/* Union with GTY marker */
union GTY(()) tagged_union {
  struct complex_struct *as_struct;  /* TYPE_POINTER */
  int as_int;                        /* TYPE_SCALAR */
  callback_type as_callback;         /* TYPE_CALLBACK */
};

/* Array of structures */
struct GTY(()) array_container {
  /* TYPE_ARRAY of structs */
  struct inner_struct items[4];
  
  /* Flexible array member (TYPE_ARRAY) */
  int flexible_array[];
};

/* Struct with callback field */
struct GTY(()) callback_container {
  /* TYPE_CALLBACK */
  int (*process)(struct complex_struct *, void *);
  
  /* TYPE_STRING */
  const char *error_msg;
};

/* Mix of GTY and non-GTY types in relationships */

/* Non-GTY struct referenced by GTY struct */
struct non_gty_helper {
  int helper_id;
  char helper_name[32];
};

/* GTY struct with pointer to non-GTY struct */
struct GTY(()) mixed_references {
  /* TYPE_POINTER to non-GTY struct */
  struct non_gty_helper *helper;
  
  /* TYPE_POINTER to GTY struct */
  struct complex_struct *complex;
  
  /* TYPE_SCALAR */
  unsigned int flags;
};

/* Additional test for edge cases */

/* Void pointer typedef */
typedef void *generic_pointer;

/* Const pointer */
typedef const int *const_int_ptr;

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Function returning pointer */
typedef struct complex_struct *(*factory_func)(int);

/* Nested array */
typedef int matrix[3][3];

/* Struct with bitfield (scalar) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Complete the opaque struct declaration from earlier */
struct opaque_struct {
  /* This was TYPE_UNDEFINED when forward declared,
     now it becomes TYPE_STRUCT when defined */
  void *data;
  int size;
};

/* Test TYPE_NONE - this should not be directly triggerable as it's
   for internal error handling, but we'll ensure all other cases are covered */

/* Final comprehensive struct using all type categories */
struct GTY(()) master_test_struct {
  /* TYPE_STRUCT (nested) */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_struct *user;  /* Pointer to user struct */
  
  /* TYPE_UNION */
  union my_union current_union;
  
  /* TYPE_POINTER (multiple) */
  void *generic_ptr;
  int *int_ptr;
  struct inner_struct **double_ptr;
  
  /* TYPE_ARRAY (multiple dimensions) */
  int grid[5][5];
  char strings[10][50];
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_specific *lang_ptr;
  
  /* TYPE_SCALAR (various) */
  short short_val;
  long long_val;
  unsigned char byte_val;
  enum my_enum choice;
  
  /* TYPE_STRING */
  const char *title;
  char *mutable_string;
  
  /* TYPE_CALLBACK */
  void (*notify)(struct master_test_struct *, int);
  factory_func create;
  
  /* TYPE_ARRAY of unions */
  union my_union union_array[4];
  
  /* TYPE_ARRAY of callbacks */
  callback_type callbacks[3];
  
  /* Flexible array member */
  struct inner_struct *flex_array[];
};
