/* test-coverage.gt - Comprehensive test file for gengtype coverage */

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
  ENUM_VALUE_1,
  ENUM_VALUE_2,
  ENUM_VALUE_3
};

/* TYPE_STRING: String types */
typedef const char *string_type;
typedef char *mutable_string_type;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, void *);
typedef void (*void_callback)(void);

/* TYPE_POINTER: Simple pointer types */
typedef int *int_ptr;
typedef void **void_ptr_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Fixed-size array types */
typedef int fixed_array_10[10];
typedef char fixed_string[256];

/* TYPE_ARRAY: Multi-dimensional array */
typedef int matrix_3x3[3][3];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
  int field1;
  float field2;
  char field3;
};

/* TYPE_UNION: Plain C union without GTY marker */
union plain_union {
  int as_int;
  float as_float;
  void *as_ptr;
  char as_char;
};

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
  void *GTY((skip)) data;
  int id;
  const char *name;
};

/* TYPE_USER_STRUCT: Another user struct with nested types */
struct GTY((user)) complex_user_struct {
  struct user_struct *GTY((tag("USER_PTR"))) user_ptr;
  union plain_union value;
  simple_callback callback;
};

/* TYPE_STRUCT: GTY-marked struct (not user) */
struct GTY(()) gty_struct {
  int GTY((skip)) regular_field;
  void * GTY((length("len"))) variable_array;
  int len;
  enum my_enum enum_field;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) gty_union {
  int int_member;
  float float_member;
  struct gty_struct *struct_ptr;
  string_type str_member;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"), desc("%0"))) lang_specific_struct {
  int lang_field;
  void *lang_data;
  complex_callback lang_callback;
};

/* TYPE_LANG_STRUCT: Another language struct with different tag */
struct GTY((tag("CPLUSPLUS"), desc("%1"))) cpp_lang_struct {
  struct lang_specific_struct *base;
  int cpp_specific;
};

/* TYPE_ARRAY: Variable-length array in GTY struct */
struct GTY(()) var_array_struct {
  int count;
  int GTY((length("%h.count"))) items[];
};

/* TYPE_POINTER: Complex pointer types in nested structures */
struct GTY(()) nested_pointer_struct {
  /* Pointer to array */
  int (*GTY((skip)) array_ptr)[10];
  
  /* Pointer to function pointer */
  simple_callback *callback_ptr;
  
  /* Pointer to union */
  union gty_union *union_ptr;
  
  /* Pointer to language struct */
  struct lang_specific_struct *lang_ptr;
};

/* TYPE_ARRAY: Array of pointers */
typedef void *pointer_array[20];

/* TYPE_ARRAY: Array of callbacks */
typedef simple_callback callback_array[5];

/* TYPE_STRUCT: Struct containing all major type kinds */
struct GTY(()) comprehensive_struct {
  /* SCALAR */
  scalar_int int_field;
  enum my_enum enum_field;
  
  /* STRING */
  string_type string_field;
  
  /* POINTER */
  int_ptr int_pointer;
  struct opaque_struct *opaque_pointer;
  
  /* ARRAY */
  fixed_array_10 fixed_array_field;
  char flexible_array[];
  
  /* CALLBACK */
  simple_callback callback_field;
  
  /* Nested STRUCT */
  struct gty_struct nested_struct;
  
  /* Nested UNION */
  union gty_union nested_union;
  
  /* Pointer to LANG_STRUCT */
  struct lang_specific_struct *lang_struct_ptr;
  
  /* USER_STRUCT pointer */
  struct user_struct *user_struct_ptr;
};

/* TYPE_UNION: Union containing various types */
union GTY(()) comprehensive_union {
  struct comprehensive_struct as_struct;
  union gty_union as_gty_union;
  struct lang_specific_struct *as_lang_ptr;
  callback_array as_callbacks;
  matrix_3x3 as_matrix;
};

/* TYPE_POINTER: Typedef for pointer to comprehensive union */
typedef union comprehensive_union *comp_union_ptr;

/* TYPE_ARRAY: Array of comprehensive structs */
typedef struct comprehensive_struct comp_struct_array[3];

/* TYPE_CALLBACK: Callback that uses many types */
typedef void (*complex_type_callback)(
  struct comprehensive_struct *,
  union comprehensive_union *,
  struct lang_specific_struct *,
  string_type,
  int
);

/* Final struct that ties everything together */
struct GTY(()) master_test_struct {
  /* Direct instances */
  struct comprehensive_struct direct_struct;
  union comprehensive_union direct_union;
  
  /* Pointers */
  comp_union_ptr union_ptr;
  struct lang_specific_struct *lang_ptr;
  struct user_struct *user_ptr;
  
  /* Arrays */
  comp_struct_struct_array array_of_structs;
  pointer_array ptr_array;
  
  /* Callbacks */
  complex_type_callback master_callback;
  simple_callback simple_cb;
  
  /* String */
  string_type description;
  
  /* Scalar */
  int master_id;
  enum my_enum master_enum;
  
  /* Variable length array */
  int var_count;
  int GTY((length("%h.var_count"))) var_data[];
};
