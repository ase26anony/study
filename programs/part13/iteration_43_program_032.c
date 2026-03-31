/* Test coverage for gengtype-state.cc switch statement cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_ENUM (handled as scalar) */
enum my_enum {
  ENUM_VALUE_1,
  ENUM_VALUE_2,
  ENUM_VALUE_3
};

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
  enum my_enum status;
};

/* TYPE_UNION: Union type */
union my_union {
  int int_value;
  float float_value;
  void *ptr_value;
  const char *string_value;
};

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct plain_struct* struct_ptr;
typedef union my_union* union_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char *string_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
  int lang_field;
  void *lang_data;
  callback_type lang_callback;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
  /* TYPE_POINTER within struct */
  struct plain_struct *plain_ptr;
  
  /* TYPE_ARRAY within struct */
  int numbers[20];
  
  /* TYPE_UNION within struct */
  union my_union data_union;
  
  /* TYPE_CALLBACK within struct */
  callback_type handler;
  
  /* TYPE_STRING within struct */
  const char *name;
  
  /* TYPE_SCALAR within struct */
  scalar_int count;
  enum my_enum current_state;
  
  /* TYPE_USER_STRUCT pointer */
  struct user_struct *user_data;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_specific *lang_data;
  
  /* Flexible array member (TYPE_ARRAY) */
  int flexible_array[];
};

/* Another union with GTY marker */
union GTY(()) tagged_union {
  struct plain_struct as_struct;
  struct user_struct *as_user_ptr;
  int as_int;
  float as_float;
};

/* Array of pointers (TYPE_ARRAY of TYPE_POINTER) */
typedef void* GTY((length("array_length"))) pointer_array[];

/* Structure containing array of callbacks */
struct GTY(()) callback_container {
  int count;
  callback_type GTY((length("count"))) handlers[];
};

/* Void pointer typedef */
typedef void *generic_pointer;

/* Structure with nested arrays */
struct GTY(()) matrix_container {
  int rows;
  int cols;
  float GTY((length("rows * cols"))) matrix[];
};

/* Union with array */
union GTY(()) array_union {
  int ints[4];
  float floats[4];
  void *ptrs[4];
};

/* Forward declaration for testing TYPE_UNDEFINED in context */
struct undefined_type;

/* Structure that references undefined type */
struct GTY(()) references_undefined {
  struct undefined_type *undef_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED */
  int valid_data;
};

/* Enumeration pointer */
typedef enum my_enum *enum_ptr;

/* Constant pointer types */
typedef const int *const_int_ptr;
typedef int *const int_ptr_const;
typedef const int *const const_int_ptr_const;

/* Structure with all basic types */
struct GTY(()) all_types_struct {
  /* SCALAR types */
  char c;
  short s;
  int i;
  long l;
  float f;
  double d;
  enum my_enum e;
  
  /* POINTER types */
  void *vp;
  int *ip;
  const char *cp;
  struct plain_struct *sp;
  
  /* ARRAY types */
  int fixed[5];
  char string[32];
  
  /* UNION type */
  union my_union u;
  
  /* CALLBACK type */
  callback_type cb;
};

/* Typedef chain for testing */
typedef int base_type;
typedef base_type derived_type;
typedef derived_type *derived_ptr;

/* Anonymous struct/union */
struct GTY(()) has_anonymous {
  struct {
    int x;
    int y;
  } point;
  
  union {
    int as_int;
    float as_float;
  } value;
};

/* Bitfield structure (scalar) */
struct bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int signed_field : 4;
};

/* Structure with variable length array based on another field */
struct GTY(()) vla_struct {
  int count;
  char description[32];
  int GTY((length("count"))) variable_array[];
};

/* Double pointer */
typedef int **double_ptr;

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Function returning pointer */
typedef int *(*func_returning_ptr)(void);

/* Structure with nested structure */
struct GTY(()) outer_struct {
  int id;
  struct GTY(()) inner_struct {
    int value;
    void *data;
  } inner;
  struct inner_struct *inner_ptr;
};

/* Complete the undefined type definition to avoid errors */
struct undefined_type {
  int defined_now;
  void *data;
};
