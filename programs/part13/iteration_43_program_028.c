/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *);

/* TYPE_ENUM: Enumeration type */
enum my_enum {
  E1,
  E2,
  E3
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
};

/* TYPE_UNION: Union type */
union my_union {
  int a;
  float b;
  void *c;
  const char *d;
};

/* TYPE_POINTER: Various pointer types */
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
};

/* Complex nested types to ensure deep processing */

/* GTY-marked struct containing various types */
struct GTY(()) complex_struct {
  /* TYPE_POINTER */
  void *ptr_field;
  
  /* TYPE_STRING */
  const char *name;
  
  /* TYPE_ARRAY (fixed size) */
  int numbers[20];
  
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
  struct plain_struct *struct_array[5];
};

/* Another GTY struct for TYPE_USER_STRUCT */
struct GTY((user)) another_user_struct {
  string_type description;
  callback_type handlers[3];
  struct complex_struct *complex;
};

/* Flexible array member (variable-length array) */
struct GTY(()) flexible_struct {
  int length;
  char flexible_array[];
};

/* Array of unions */
union GTY(()) union_array_container {
  union my_union unions[8];
};

/* Struct with pointer to callback */
struct GTY(()) callback_container {
  callback_type action;
  void *context;
};

/* Nested pointer types */
typedef struct complex_struct **double_ptr;
typedef callback_type (*meta_callback)(callback_type);

/* Void pointer typedef */
typedef void *generic_ptr;

/* Const pointer typedef */
typedef const int *const_int_ptr;

/* Struct with bitfield (scalar) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Union with struct member */
union GTY(()) mixed_union {
  struct plain_struct s;
  struct complex_struct *cs;
  callback_type cb;
};

/* Array of function pointers */
typedef void (*func_ptr_array[5])(int, const char *);

/* Struct containing array of structs */
struct GTY(()) array_of_structs {
  struct plain_struct items[10];
  int count;
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* Typedef for undefined struct */
typedef struct never_defined *undefined_ptr;

/* Empty struct */
struct GTY(()) empty_struct {
  /* intentionally empty */
};

/* Struct with all scalar types */
struct GTY(()) all_scalars {
  char c;
  short s;
  int i;
  long l;
  long long ll;
  float f;
  double d;
  _Bool b;
  enum my_enum e;
};

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Complex nested example */
struct GTY(()) outermost {
  struct GTY(()) middle {
    struct GTY(()) innermost {
      int value;
      void *data;
    } *inner;
    
    union my_union u;
    callback_type cb;
  } mid;
  
  string_type name;
  struct array_of_structs aos;
};

/* Additional TYPE_LANG_STRUCT with different tag */
struct GTY((tag("OTHER_LANG"))) another_lang_struct {
  int other_field;
  struct lang_specific *related;
};

/* Struct with pointer to lang struct */
struct GTY(()) lang_container {
  struct lang_specific *lang_data;
  struct another_lang_struct *other_lang_data;
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
  int defined_later;
  void *data;
};

/* Union containing string */
union GTY(()) string_union {
  string_type str;
  char chars[100];
};

/* Array of strings */
typedef string_type multi_string[20];

/* Struct with multiple pointer types */
struct GTY(()) pointer_collection {
  void *vp;
  int *ip;
  float *fp;
  struct plain_struct *sp;
  union my_union *up;
  callback_type *cpp;
  const char **ssp;
};

/* Final catch-all structure containing one of everything */
struct GTY(()) everything_bagel {
  /* TYPE_SCALAR */
  int scalar_int;
  enum my_enum scalar_enum;
  
  /* TYPE_STRING */
  const char *scalar_string;
  
  /* TYPE_POINTER */
  void *scalar_pointer;
  int *int_pointer;
  struct everything_bagel *self_pointer;
  
  /* TYPE_ARRAY */
  int scalar_array[5];
  void *pointer_array[3];
  
  /* TYPE_STRUCT */
  struct plain_struct embedded_struct;
  
  /* TYPE_UNION */
  union my_union embedded_union;
  
  /* TYPE_CALLBACK */
  callback_type embedded_callback;
  
  /* TYPE_USER_STRUCT */
  struct user_struct *user_struct_ptr;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific *lang_struct_ptr;
  
  /* TYPE_UNDEFINED (through pointer) */
  struct never_defined *undefined_ptr;
  
  /* Flexible array member */
  char trailing_data[];
};
